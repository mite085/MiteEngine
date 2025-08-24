#include "hierarchy_component.h"
#include "transform_component.h"

namespace mite {
HierarchyComponent::HierarchyComponent() : ComponentTraits() {}

void HierarchyComponent::ProcessDirty(float deltaTime, SceneRegistry &reg) {
  if (!IsDirty())
    return;

  // 标记相关的TransformComponent需要更新
  UpdateTransformDirtyState(reg);

  ClearDirty();
}
Entity HierarchyComponent::GetParent() const
{
  return m_Parent;
}
const std::vector<Entity> &HierarchyComponent::GetChildren() const
{
  return m_Children;
}
size_t HierarchyComponent::GetChildCount() const
{
  return m_Children.size();
}
bool HierarchyComponent::IsLeaf() const
{
  return m_Children.empty();
}
bool HierarchyComponent::IsRoot() const
{
  return m_Parent == Entity();
}
size_t HierarchyComponent::GetDepth(SceneRegistry &registry)
{
  // 如果已经是根节点，深度为0
  if (IsRoot()) {
    return 0;
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
      break;  // 父实体没有层次组件，判断当前为递归终点
    }
    current = parentHierarchy->GetParent();
    ++depth;

    // 防止无限循环
    if (depth > 1000)
      break;
  }

  return depth;
}

bool HierarchyComponent::SetParent(SceneRegistry &registry, Entity newParent)
{
  if (!ValidateHierarchy(registry, newParent)) {
    return false;
  }

  Entity oldParent = m_Parent;

  // 从旧父节点移除
  if (oldParent.IsValid() && registry.HasComponent<HierarchyComponent>(oldParent)) {
    auto &oldParentHierarchy = registry.GetComponent<HierarchyComponent>(oldParent);
    oldParentHierarchy.RemoveChild(registry, GetOwnerEntity());
  }

  // 设置新父节点
  m_Parent = newParent;

  // 添加到新父节点
  if (newParent.IsValid() && registry.HasComponent<HierarchyComponent>(newParent)) {
    auto &newParentHierarchy = registry.GetComponent<HierarchyComponent>(newParent);
    newParentHierarchy.AddChild(registry, GetOwnerEntity());
  }

  MarkDirty();

  // 发布事件
  EventBus::Get().Post(ParentChangedEvent(GetOwnerEntity(), *this, oldParent, newParent));

  return true;
}

bool HierarchyComponent::AddChild(SceneRegistry &registry, Entity child)
{
  if (!child.IsValid() || !registry.IsValid(child)) {
    return false;
  }

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return true;  // 已经是子节点，不算失败
  }

  // 设置子节点的父节点
  if (registry.HasComponent<HierarchyComponent>(child)) {
    auto &childHierarchy = registry.GetComponent<HierarchyComponent>(child);
    if (!childHierarchy.SetParent(registry, GetOwnerEntity())) {
      return false;
    }
  }

  // 设置子节点
  m_Children.push_back(child);
  MarkDirty();

  return true;
}

bool HierarchyComponent::RemoveChild(SceneRegistry &registry, Entity child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it == m_Children.end()) {
    return false;
  }

  // 清除子节点的父节点
  if (registry.HasComponent<HierarchyComponent>(child)) {
    auto &childHierarchy = registry.GetComponent<HierarchyComponent>(child);
    childHierarchy.SetParent(registry, Entity());
  }

  m_Children.erase(it);
  MarkDirty();

  return true;
}

void HierarchyComponent::ClearChildren(SceneRegistry &registry)
{
  for (auto child : m_Children) {
    if (registry.HasComponent<HierarchyComponent>(child)) {
      auto &childHierarchy = registry.GetComponent<HierarchyComponent>(child);
      childHierarchy.SetParent(registry, Entity());
    }
  }

  m_Children.clear();
  MarkDirty();
}

bool HierarchyComponent::ValidateHierarchy(SceneRegistry &registry, Entity newParent) const
{
  // 检查自引用
  if (newParent == GetOwnerEntity()) {
    return false;
  }

  // 检查循环引用
  Entity current = newParent;
  while (current.IsValid() && registry.IsValid(current)) {
    if (current == GetOwnerEntity()) {
      return false;
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

void HierarchyComponent::UpdateTransformDirtyState(SceneRegistry &registry)
{
  // 标记自己的TransformComponent需要更新
  if (registry.HasComponent<TransformComponent>(GetOwnerEntity())) {
    auto &transform = registry.GetComponent<TransformComponent>(GetOwnerEntity());
    transform.MarkDirty();
  }

  // 递归标记所有子节点的TransformComponent
  for (auto child : m_Children) {
    if (registry.HasComponent<HierarchyComponent>(child)) {
      auto &childHierarchy = registry.GetComponent<HierarchyComponent>(child);
      childHierarchy.UpdateTransformDirtyState(registry);
    }
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
  // 验证并修复所有层级关系的完整性
  ValidateAndRepairHierarchy(registry);

  // 处理脏组件
  for (auto *comp : m_DirtyComponents) {
    comp->ProcessDirty(deltaTime, registry);
    comp->ClearDirty();
  }

  // 清空脏组件列表
  m_DirtyComponents.clear();
}

bool HierarchyComponentSystem::OnComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e)
{
  auto &oldComponent = e.GetComponent();
  Entity entity = e.GetEntity();

  // 1. 从父节点中移除自己
  if (oldComponent.GetParent().IsValid()) {
    if (GetRegistry().HasComponent<HierarchyComponent>(oldComponent.GetParent())) {
      auto &parentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
          oldComponent.GetParent());
      parentHierarchy.RemoveChild(GetRegistry(), entity);
    }
  }

  // 2. 清除所有子节点的父节点
  for (auto child : oldComponent.GetChildren()) {
    if (GetRegistry().HasComponent<HierarchyComponent>(child)) {
      auto &childHierarchy = GetRegistry().GetComponent<HierarchyComponent>(child);
      childHierarchy.SetParent(GetRegistry(), Entity());
    }
  }

  // 标记事件已处理，阻断传播
  e.Handled();
  return e.handled;
}

void HierarchyComponentSystem::ValidateAndRepairHierarchy(SceneRegistry &registry)
{
  auto view = registry.GetEntitiesWith<HierarchyComponent>();
  std::vector<std::pair<Entity, Entity>> invalidRelations;

  // 验证所有层级关系的完整性
  for (auto entity : view) {
    auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);

    // 验证父节点，仅当parent存在，但Invalid或者没有Hierarchy组件时，认为需要修复
    Entity parent = hierarchy.GetParent();
    if (parent.IsValid() &&
        (!registry.IsValid(parent) || !registry.HasComponent<HierarchyComponent>(parent)))
    {
      invalidRelations.emplace_back(entity, parent);
    }

    // 验证子节点，仅当child存在，但Invalid或者没有Hierarchy组件时，认为需要修复
    for (auto child : hierarchy.GetChildren()) {
      if (!registry.IsValid(child) || !registry.HasComponent<HierarchyComponent>(child)) {
        invalidRelations.emplace_back(entity, child);
      }
    }
  }

  // 使用HierarchyComponent接口修复
  for (auto &[entity, invalidRef] : invalidRelations) {
    auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);

    if (invalidRef == hierarchy.GetParent()) {
      hierarchy.SetParent(registry, Entity());
    }
    else {
      hierarchy.RemoveChild(registry, invalidRef);
    }
  }
}

};  // namespace mite