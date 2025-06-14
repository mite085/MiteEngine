#include "scene_graph.h"
#include "scene_core_components/hierarchy_component.h"

namespace mite {
bool SceneGraph::SetParent(entt::entity entity, entt::entity newParent)
{
  // 检查实体有效性
  if (!m_Registry.valid(entity)) {
    return false;
  }

  // 如果新父节点不为null，检查其有效性
  if (newParent != entt::null && !m_Registry.valid(newParent)) {
    return false;
  }

  // 检查是否设置为自己父节点
  if (entity == newParent) {
    return false;
  }

  // 检查是否形成循环依赖
  if (newParent != entt::null && WouldFormCycle(entity, newParent)) {
    return false;
  }

  // 获取当前父节点
  auto &hierarchy = m_Registry.get_or_emplace<HierarchyComponent>(entity);
  entt::entity oldParent = hierarchy.GetParent();

  // 如果父节点没有变化，直接返回成功
  if (oldParent == newParent) {
    return true;
  }

  // 从旧父节点中移除
  if (oldParent != entt::null) {
    auto &oldParentHierarchy = m_Registry.get<HierarchyComponent>(oldParent);
    oldParentHierarchy.RemoveChild(entity);
  }

  // 设置新父节点
  hierarchy.SetParent(newParent);

  // 添加到新父节点的子列表
  if (newParent != entt::null) {
    auto &newParentHierarchy = m_Registry.get_or_emplace<HierarchyComponent>(newParent);
    newParentHierarchy.AddChild(entity);
  }

  // 重新计算受影响实体的深度
  // 这里可以优化为只更新子树的深度
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
  return true;  // 没有HierarchyComponent的实体视为根节点
}

bool SceneGraph::IsLeaf(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->IsLeaf();
  }
  return true;  // 没有HierarchyComponent的实体视为叶节点
}

size_t SceneGraph::GetDepth(entt::entity entity) const
{
  if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
    return hierarchy->GetDepth(m_Registry);
  }
  return 0;  // 没有HierarchyComponent的实体深度为0
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
  // 获取两个实体到根节点的路径
  auto path1 = GetPathToRoot(entity1);
  auto path2 = GetPathToRoot(entity2);

  // 检查是否有共同节点
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

  // 遍历所有有HierarchyComponent的实体，收集根节点
  auto &storage = m_Registry.storage<entt::entity>();
  for (auto entity : storage) {
    if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
      if (hierarchy->IsRoot()) {
        roots.push_back(entity);
      }
    }
    else {
      // 没有HierarchyComponent的实体也视为根节点
      roots.push_back(entity);
    }
  }

  return roots;
}

void SceneGraph::RecalculateAllDepths()
{
  // 广度优先遍历所有根节点，计算深度
  std::queue<std::pair<entt::entity, size_t>> queue;

  // 初始队列包含所有根节点，深度为0
  auto roots = GetRoots();
  for (auto root : roots) {
    queue.emplace(root, 0);
  }

  while (!queue.empty()) {
    auto [entity, depth] = queue.front();
    queue.pop();

    if (auto *hierarchy = m_Registry.try_get<HierarchyComponent>(entity)) {
      hierarchy->m_DepthCache = depth;

      // 将子节点加入队列，深度+1
      for (auto child : hierarchy->GetChildren()) {
        queue.emplace(child, depth + 1);
      }
    }
  }
}

void SceneGraph::OnUpdate(float timestep)
{  
  // 1. 处理变换继承 --------------------------------------------

  // 使用EnTT的多线程视图(如果可用)并行处理变换更新
  auto view = m_Registry.view<TransformComponent, HierarchyComponent>();

  // 第一遍: 收集所有脏标记的根变换
  std::vector<entt::entity> dirtyRoots;
  for (auto entity : view) {
    auto &transform = view.get<TransformComponent>(entity);
    auto &hierarchy = view.get<HierarchyComponent>(entity);

    // 如果是根节点或者父节点没有脏标记，但自身有脏标记
    if (hierarchy.IsRoot() && transform.IsDirty()) {
      dirtyRoots.push_back(entity);
    }
    else if (!hierarchy.IsRoot()) {
      auto parent = hierarchy.GetParent();
      if (m_Registry.valid(parent)) {
        if (auto parentTransform = m_Registry.try_get<TransformComponent>(parent)) {
          // 如果父节点有脏标记，子节点也需要更新
          if (parentTransform->IsDirty() || transform.IsDirty()) {
            dirtyRoots.push_back(entity);
          }
        }
      }
    }
  }

  // 第二遍: 从脏标记根节点开始向下传播变换
  for (auto root : dirtyRoots) {
    Traverse(
        root,
        [this](entt::entity entity) {
          if (auto transform = m_Registry.try_get<TransformComponent>(entity)) {
            // 获取父节点变换(如果是根节点则为单位矩阵)
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

            // 更新世界变换
            transform->UpdateWorldTransform(parentTransform);

            // 清除脏标记
            transform->ClearDirtyFlag();
          }
          return true;  // 继续遍历
        },
        TraversalOrder::DepthFirst);
  }

  // 2. 处理可见性继承 ------------------------------------------

  // 只有在前一阶段有变换更新时才需要更新可见性
  if (!dirtyRoots.empty()) {
    auto visibilityView = m_Registry.view<VisibilityComponent, HierarchyComponent>();

    for (auto entity : visibilityView) {
      auto &visibility = visibilityView.get<VisibilityComponent>(entity);
      auto &hierarchy = visibilityView.get<HierarchyComponent>(entity);

      // 如果父节点不可见，子节点也不可见
      if (!hierarchy.IsRoot()) {
        auto parent = hierarchy.GetParent();
        if (m_Registry.valid(parent)) {
          if (auto parentVisibility = m_Registry.try_get<VisibilityComponent>(parent)) {
            if (!parentVisibility->IsVisible()) {
              visibility.SetVisible(false, false);  // 不传播，避免重复处理
            }
          }
        }
      }
    }
  }

  // 3. 处理延迟的层次结构变更 ----------------------------------

  //// TODO: 如果有延迟的父子关系变更，在这里处理
  //ProcessDeferredHierarchyChanges();

  // 4. 更新场景图统计信息 --------------------------------------

  //// TODO: 按照时间step更新统计信息
  //UpdateSceneStatistics(timestep);
}

bool SceneGraph::WouldFormCycle(entt::entity child, entt::entity newParent) const
{
  // 从新父节点向上遍历，如果遇到child则形成循环
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
