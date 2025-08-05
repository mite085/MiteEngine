#include "hierarchy_component.h"

namespace mite {
HierarchyComponent::HierarchyComponent() : ComponentTraits() {}

HierarchyComponent::HierarchyComponent(const HierarchyComponent &other) noexcept
    : m_Parent(other.m_Parent),
      m_Children(other.m_Children),
      m_DepthCache(0)  // 深度缓存重置（需要重新计算）
{
}
HierarchyComponent &HierarchyComponent::operator=(const HierarchyComponent &other) noexcept
{
  if (this != &other) {
    m_Parent = other.m_Parent;
    m_Children = other.m_Children;
    m_DepthCache = 0;  // 深度缓存失效
  }
  return *this;
}
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
  if (parent.IsValid()) {
    m_Parent = parent;  // 有效parent，正常赋值
    m_DepthCache = 0;   // 使深度缓存失效
  }
  else {
    m_Parent.Destroy();  // 无效parent，将parent置空
    m_DepthCache = 0;    // 使深度缓存失效
  }
}

void HierarchyComponentSystem::Initialize(SceneRegistry &registry)
{
  DirtyComponentSystem<HierarchyComponent>::Initialize(registry);
}

void HierarchyComponentSystem::Shutdown(SceneRegistry &registry)
{
  DirtyComponentSystem<HierarchyComponent>::Shutdown(registry);
}

void HierarchyComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 处理所有脏组件
  for (auto *comp : m_DirtyComponents) {
    // 使深度缓存失效
    comp->m_DepthCache = 0;

    // 获取实体
    Entity entity = comp->GetOwnerEntity();

    // 更新该实体及其所有子代的深度缓存
    UpdateChildrenDepthCache(entity, registry);

    // 清除脏标记
    comp->CleanDirty();
  }
  // 清空脏组件列表
  m_DirtyComponents.clear();
}

void HierarchyComponentSystem::OnComponentUpdated(ComponentChangedEvent<HierarchyComponent> &e)
{
  auto &hierarchy = e.GetComponent();
  Entity entity = e.GetEntity();
  Entity oldParent = e.GetOldComponent().GetParent();
  Entity newParent = hierarchy.GetParent();

  // 验证新的父子关系是否有效
  if (!ValidateHierarchy(entity, newParent, GetRegistry())) {
    // 如果无效，恢复原来的父节点
    hierarchy.SetParent(oldParent);
    return;
  }

  // 1. 从旧父节点中移除
  if (oldParent.IsValid() && GetRegistry().IsValid(oldParent)) {
    if (GetRegistry().HasComponent<HierarchyComponent>(oldParent)) {
      auto &oldParentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(oldParent);
      oldParentHierarchy.RemoveChild(entity);

      // 发布子节点移除事件
      EventBus::Get().Post(ChildRemovedEvent(oldParent, oldParentHierarchy, entity));
    }
  }

  // 2. 添加到新父节点
  if (newParent.IsValid() && GetRegistry().IsValid(newParent)) {
    if (GetRegistry().HasComponent<HierarchyComponent>(newParent)) {
      auto &newParentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(newParent);
      newParentHierarchy.AddChild(entity);

      // 发布子节点添加事件
      EventBus::Get().Post(ChildAddedEvent(newParent, newParentHierarchy, entity));
    }
  }

  // 发布父节点改变事件
  EventBus::Get().Post(ParentChangedEvent(entity, hierarchy, oldParent, newParent));

  // 注册组件为脏
  Register(&hierarchy);
  hierarchy.MarkDirty();
}

void HierarchyComponentSystem::OnComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e)
{
  auto &oldComponent = e.GetComponent();
  Unregister(&oldComponent);

  // 处理父节点和子节点的关系
  Entity entity = e.GetEntity();

  // 1. 从父节点中移除自己
  if (oldComponent.GetParent().IsValid() && GetRegistry().IsValid(oldComponent.GetParent())) {
    if (GetRegistry().HasComponent<HierarchyComponent>(oldComponent.GetParent())) {
      auto &parentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
          oldComponent.GetParent());
      parentHierarchy.RemoveChild(entity);

      // 发布子节点移除事件
      EventBus::Get().Post(ChildRemovedEvent(oldComponent.GetParent(), parentHierarchy, entity));
    }
  }

  // 2. 将所有子节点的父节点设为空
  for (Entity child : oldComponent.GetChildren()) {
    if (GetRegistry().IsValid(child) && GetRegistry().HasComponent<HierarchyComponent>(child)) {
      auto &childHierarchy = GetRegistry().GetComponent<HierarchyComponent>(child);
      childHierarchy.SetParent(Entity());

      // 发布父节点改变事件
      EventBus::Get().Post(ParentChangedEvent(child, childHierarchy, entity, Entity()));
    }
  }
}

bool HierarchyComponentSystem::ValidateHierarchy(Entity entity,
                                                 Entity newParent,
                                                 SceneRegistry &registry)
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

void HierarchyComponentSystem::UpdateChildrenDepthCache(Entity entity, SceneRegistry &registry)
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
    }
  }
}
};  // namespace mite