#include "hierarchy_component.h"
#include "transform_component.h"

namespace mite {
HierarchyComponent::HierarchyComponent() : ComponentTraits() {}
size_t HierarchyComponent::GetDepth(SceneRegistry &registry)
{
  // 如果已经是根节点，深度为0
  if (IsRoot()) {
    return 0;
  }

  // 检查缓存有效性
  if (m_DepthCache > 0) {
    return m_DepthCache;
  }

  // 递归计算深度
  size_t depth = 0;
  Entity current = m_Parent;

  while (current.IsValid()) {
    if (!registry.IsValid(current)) {
      break;  // 遇到无效实体终止
    }

    auto parentHierarchy = registry.TryGetComponent<HierarchyComponent>(current);
    if (!parentHierarchy) {
      break;  // 父实体没有层次组件
    }

    ++depth;
    current = parentHierarchy->GetParent();
  }

  // 更新缓存（注意：缓存只在计算期间有效，不持久化）
  m_DepthCache = depth;

  return depth;
}

void HierarchyComponent::AddChild(Entity child)
{
  // Error check: Cannot add null entity as child!
  assert(child.IsValid());

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  m_Children.push_back(child);
  m_DepthCache = 0;  // 使深度缓存失效
}

bool HierarchyComponent::RemoveChild(Entity child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
    m_DepthCache = 0;  // 使深度缓存失效
    return true;
  }
  return false;
}

void HierarchyComponent::ClearChildren()
{
  m_Children.clear();
  m_DepthCache = 0;  // 使深度缓存失效
}

void HierarchyComponent::SetParent(Entity parent)
{
  m_Parent = parent;
  m_DepthCache = 0;  // 使深度缓存失效
}

void HierarchySystem::Initialize(SceneRegistry &registry)
{
  m_Registry = registry;

  // 订阅相关事件
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnComponentAdded));
  m_EventSubscriptions.Subscribe<ComponentChangedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnComponentUpdated));
  m_EventSubscriptions.Subscribe<ComponentRemovedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnComponentRemoved));
}

void HierarchySystem::Shutdown(SceneRegistry &registry)
{
  m_EventSubscriptions.UnsubscribeAll();
  m_AllComponents.clear();
  m_DirtyComponents.clear();
}

void HierarchySystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 处理所有脏组件
  for (auto *comp : m_DirtyComponents) {
    // 使深度缓存失效
    comp->m_DepthCache = 0;

    // 获取实体
    Entity entity = comp->GetOwnerEntity();

    // 更新该实体及其所有子代的深度缓存
    UpdateChildrenDepthCache(entity, registry);

    // 标记Transform组件为脏，以便更新变换
    if (registry.HasComponent<TransformComponent>(entity)) {
      auto &transform = registry.GetComponent<TransformComponent>(entity);
      transform.dirtyFlags |= TransformComponent::HIERARCHY_DIRTY;
      transform.MarkDirty();
    }

    // 清除脏标记
    comp->CleanDirty();
  }
}

void HierarchySystem::OnComponentUpdated(ComponentChangedEvent<HierarchyComponent> &e)
{
  auto &hierarchy = e.GetComponent();
  Entity entity = e.GetEntity();

  // 验证新的父子关系是否有效
  if (!ValidateHierarchy(entity, hierarchy.GetParent(), *m_Registry)) {
    // 如果无效，恢复原来的父节点
    hierarchy.SetParent(e.GetOldComponent().GetParent());
    return;
  }

  // 注册组件为脏
  Register(&hierarchy);
  hierarchy.MarkDirty();
}

bool HierarchySystem::ValidateHierarchy(Entity entity, Entity newParent, SceneRegistry &registry)
{
  // 不允许设置自己为自己的父节点
  if (entity == newParent) {
    return false;
  }

  // 检查循环依赖
  Entity current = newParent;
  while (current.IsValid() && registry.IsValid(current)) {
    if (current == entity) {
      return false;  // 检测到循环
    }

    if (registry.HasComponent<HierarchyComponent>(current)) {
      auto &hierarchy = registry.GetComponent<HierarchyComponent>(current);
      current = hierarchy.GetParent();
    }
    else {
      break;
    }
  }

  return true;
}

void HierarchySystem::UpdateChildrenDepthCache(Entity entity, SceneRegistry &registry)
{
  if (!registry.IsValid(entity) || !registry.HasComponent<HierarchyComponent>(entity)) {
    return;
  }

  auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);

  // 使当前实体的深度缓存失效
  hierarchy.m_DepthCache = 0;

  // 递归处理所有子实体
  for (Entity child : hierarchy.GetChildren()) {
    if (registry.IsValid(child) && registry.HasComponent<HierarchyComponent>(child)) {
      UpdateChildrenDepthCache(child, registry);

      // 标记子实体的Transform组件为脏
      if (registry.HasComponent<TransformComponent>(child)) {
        auto &transform = registry.GetComponent<TransformComponent>(child);
        transform.dirtyFlags |= TransformComponent::HIERARCHY_DIRTY;
        transform.MarkDirty();
      }
    }
  }
}

};
