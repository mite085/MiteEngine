#include "scene_graph.h"
#include "scene_core_components/hierarchy_component.h"

namespace mite {
bool SceneGraph::SetParent(entt::entity entity, entt::entity newParent)
{
  // ���ʵ����Ч��
  if (!m_Registry.valid(entity)) {
    return false;
  }

  // ����¸��ڵ㲻Ϊnull���������Ч��
  if (newParent != entt::null && !m_Registry.valid(newParent)) {
    return false;
  }

  // ����Ƿ�����Ϊ�Լ����ڵ�
  if (entity == newParent) {
    return false;
  }

  // ����Ƿ��γ�ѭ������
  if (newParent != entt::null && WouldFormCycle(entity, newParent)) {
    return false;
  }

  // ��ȡ��ǰ���ڵ�
  auto &hierarchy = m_Registry.get_or_emplace<HierarchyComponent>(entity);
  entt::entity oldParent = hierarchy.GetParent();

  // ������ڵ�û�б仯��ֱ�ӷ��سɹ�
  if (oldParent == newParent) {
    return true;
  }

  // �Ӿɸ��ڵ����Ƴ�
  if (oldParent != entt::null) {
    auto &oldParentHierarchy = m_Registry.get<HierarchyComponent>(oldParent);
    oldParentHierarchy.RemoveChild(entity);
  }

  // �����¸��ڵ�
  hierarchy.SetParent(newParent);

  // ��ӵ��¸��ڵ�����б�
  if (newParent != entt::null) {
    auto &newParentHierarchy = m_Registry.get_or_emplace<HierarchyComponent>(newParent);
    newParentHierarchy.AddChild(entity);
  }

  // ���¼�����Ӱ��ʵ������
  // ��������Ż�Ϊֻ�������������
  RecalculateAllDepths();

  return true;
}

entt::entity SceneGraph::GetParent(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->GetParent();
  }
  return entt::null;
}

const std::vector<entt::entity> &SceneGraph::GetChildren(entt::entity entity) const
{
  static const std::vector<entt::entity> emptyChildren;

  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->GetChildren();
  }
  return emptyChildren;
}

bool SceneGraph::IsRoot(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->IsRoot();
  }
  return true;  // û��HierarchyComponent��ʵ����Ϊ���ڵ�
}

bool SceneGraph::IsLeaf(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->IsLeaf();
  }
  return true;  // û��HierarchyComponent��ʵ����ΪҶ�ڵ�
}

size_t SceneGraph::GetDepth(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->GetDepth(m_Registry);
  }
  return 0;  // û��HierarchyComponent��ʵ�����Ϊ0
}

void SceneGraph::Traverse(entt::entity root,
                          const VisitorFunc &visitor,
                          TraversalOrder order) const
{
  if (!m_Registry.valid(root) || !visitor) {
    return;
  }

  switch (order) {
    case TraversalOrder::DepthFirst:
      TraverseDFS(root, visitor);
      break;
    case TraversalOrder::BreadthFirst:
      TraverseBFS(root, visitor);
      break;
    case TraversalOrder::ReverseDepthFirst:
      TraverseReverseDFS(root, visitor);
      break;
  }
}

void SceneGraph::TraverseAll(const VisitorFunc &visitor, TraversalOrder order) const
{
  auto roots = GetRoots();
  for (auto root : roots) {
    Traverse(root, visitor, order);
  }
}

std::vector<entt::entity> SceneGraph::GetPathToRoot(entt::entity entity) const
{
  std::vector<entt::entity> path;

  while (m_Registry.valid(entity)) {
    path.push_back(entity);
    entity = GetParent(entity);
  }

  return path;
}

bool SceneGraph::IsInSameHierarchy(entt::entity entity1, entt::entity entity2) const
{
  // ��ȡ����ʵ�嵽���ڵ��·��
  auto path1 = GetPathToRoot(entity1);
  auto path2 = GetPathToRoot(entity2);

  // ����Ƿ��й�ͬ�ڵ�
  for (auto e1 : path1) {
    if (std::find(path2.begin(), path2.end(), e1) != path2.end()) {
      return true;
    }
  }

  return false;
}

std::vector<entt::entity> SceneGraph::GetRoots() const
{
  std::vector<entt::entity> roots;

  // ����������HierarchyComponent��ʵ�壬�ռ����ڵ�
  auto &storage = m_Registry.storage<entt::entity>();
  for (auto entity : storage) {
    if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
      if (hierarchy->IsRoot()) {
        roots.push_back(entity);
      }
    }
    else {
      // û��HierarchyComponent��ʵ��Ҳ��Ϊ���ڵ�
      roots.push_back(entity);
    }
  }

  return roots;
}

void SceneGraph::RecalculateAllDepths()
{
  // ������ȱ������и��ڵ㣬�������
  std::queue<std::pair<entt::entity, size_t>> queue;

  // ��ʼ���а������и��ڵ㣬���Ϊ0
  auto roots = GetRoots();
  for (auto root : roots) {
    queue.emplace(root, 0);
  }

  while (!queue.empty()) {
    auto [entity, depth] = queue.front();
    queue.pop();

    if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
      hierarchy->m_DepthCache = depth;

      // ���ӽڵ������У����+1
      for (auto child : hierarchy->GetChildren()) {
        queue.emplace(child, depth + 1);
      }
    }
  }
}

void SceneGraph::OnUpdate(float timestep)
{  
  // 1. ����任�̳� --------------------------------------------

  // ʹ��EnTT�Ķ��߳���ͼ(�������)���д���任����
  auto view = m_Registry.view<TransformComponent, HierarchyComponent>();

  // ��һ��: �ռ��������ǵĸ��任
  std::vector<entt::entity> dirtyRoots;
  for (auto entity : view) {
    auto &transform = view.get<TransformComponent>(entity);
    auto &hierarchy = view.get<HierarchyComponent>(entity);

    // ����Ǹ��ڵ���߸��ڵ�û�����ǣ�������������
    if (hierarchy.IsRoot() && transform.IsDirty()) {
      dirtyRoots.push_back(entity);
    }
    else if (!hierarchy.IsRoot()) {
      auto parent = hierarchy.GetParent();
      if (m_Registry.valid(parent)) {
        if (auto parentTransform = m_Registry.try_get<TransformComponent>(parent)) {
          // ������ڵ������ǣ��ӽڵ�Ҳ��Ҫ����
          if (parentTransform->IsDirty() || transform.IsDirty()) {
            dirtyRoots.push_back(entity);
          }
        }
      }
    }
  }

  // �ڶ���: �����Ǹ��ڵ㿪ʼ���´����任
  for (auto root : dirtyRoots) {
    Traverse(
        root,
        [this](entt::entity entity) {
          if (auto transform = m_Registry.try_get<TransformComponent>(entity)) {
            // ��ȡ���ڵ�任(����Ǹ��ڵ���Ϊ��λ����)
            glm::mat4 parentTransform = glm::mat4(1.0f);
            if (auto hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
              if (!hierarchy->IsRoot()) {
                auto parent = hierarchy->GetParent();
                if (m_Registry.valid(parent)) {
                  if (auto parentTransformComp = m_Registry.try_get<TransformComponent>(parent)) {
                    parentTransform = parentTransformComp->GetWorldTransform();
                  }
                }
              }
            }

            // ��������任
            transform->UpdateWorldTransform(parentTransform);

            // �������
            transform->ClearDirtyFlag();
          }
          return true;  // ��������
        },
        TraversalOrder::DepthFirst);
  }

  // 2. ����ɼ��Լ̳� ------------------------------------------

  // ֻ����ǰһ�׶��б任����ʱ����Ҫ���¿ɼ���
  if (!dirtyRoots.empty()) {
    auto visibilityView = m_Registry.view<VisibilityComponent, HierarchyComponent>();

    for (auto entity : visibilityView) {
      auto &visibility = visibilityView.get<VisibilityComponent>(entity);
      auto &hierarchy = visibilityView.get<HierarchyComponent>(entity);

      // ������ڵ㲻�ɼ����ӽڵ�Ҳ���ɼ�
      if (!hierarchy.IsRoot()) {
        auto parent = hierarchy.GetParent();
        if (m_Registry.valid(parent)) {
          if (auto parentVisibility = m_Registry.try_get<VisibilityComponent>(parent)) {
            if (!parentVisibility->IsVisible()) {
              visibility.SetVisible(false, false);  // �������������ظ�����
            }
          }
        }
      }
    }
  }

  // 3. �����ӳٵĲ�νṹ��� ----------------------------------

  //// TODO: ������ӳٵĸ��ӹ�ϵ����������ﴦ��
  //ProcessDeferredHierarchyChanges();

  // 4. ���³���ͼͳ����Ϣ --------------------------------------

  //// TODO: ����ʱ��step����ͳ����Ϣ
  //UpdateSceneStatistics(timestep);
}

bool SceneGraph::WouldFormCycle(entt::entity child, entt::entity newParent) const
{
  // ���¸��ڵ����ϱ������������child���γ�ѭ��
  entt::entity current = newParent;
  while (current != entt::null) {
    if (current == child) {
      return true;
    }
    current = GetParent(current);
  }
  return false;
}

bool SceneGraph::TraverseDFS(entt::entity entity, const VisitorFunc &visitor) const
{
  if (!visitor(entity)) {
    return false;
  }

  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    for (auto child : hierarchy->GetChildren()) {
      if (!TraverseDFS(child, visitor)) {
        return false;
      }
    }
  }

  return true;
}

void SceneGraph::TraverseBFS(entt::entity entity, const VisitorFunc &visitor) const
{
  std::queue<entt::entity> queue;
  queue.push(entity);

  while (!queue.empty()) {
    auto current = queue.front();
    queue.pop();

    if (!visitor(current)) {
      return;
    }

    if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(current)) {
      for (auto child : hierarchy->GetChildren()) {
        queue.push(child);
      }
    }
  }
}

bool SceneGraph::TraverseReverseDFS(entt::entity entity, const VisitorFunc &visitor) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    for (auto child : hierarchy->GetChildren()) {
      if (!TraverseReverseDFS(child, visitor)) {
        return false;
      }
    }
  }

  return visitor(entity);
}
};
