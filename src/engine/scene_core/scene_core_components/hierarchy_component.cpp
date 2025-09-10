#include "hierarchy_component.h"
#include "transform_component.h"
#include "id_component.h"

namespace mite {
HierarchyComponent::HierarchyComponent() : ComponentTraits() {}

void HierarchyComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
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
    oldParentHierarchy.RemoveChild(registry, GetEntity());
  }

  // 设置新父节点
  m_Parent = newParent;

  // 添加到新父节点
  if (newParent.IsValid() && registry.HasComponent<HierarchyComponent>(newParent)) {
    auto &newParentHierarchy = registry.GetComponent<HierarchyComponent>(newParent);
    newParentHierarchy.AddChild(registry, GetEntity());
  }

  MarkDirty();

  // 发布事件
  EventBus::Publish<ParentChangedEvent>(
      ParentChangedEvent(GetEntity(), *this, oldParent, newParent));

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
    if (!childHierarchy.SetParent(registry, GetEntity())) {
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
  if (newParent == GetEntity()) {
    return false;
  }

  // 检查循环引用
  Entity current = newParent;
  while (current.IsValid() && registry.IsValid(current)) {
    if (current == GetEntity()) {
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
  if (registry.HasComponent<TransformComponent>(GetEntity())) {
    auto &transform = registry.GetComponent<TransformComponent>(GetEntity());
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

void HierarchyComponentSystem::Initialize()
{
  DirtyComponentSystem<HierarchyComponent>::Initialize();
}

void HierarchyComponentSystem::Shutdown()
{
  // 清空待处理队列
  {
    std::lock_guard<std::mutex> lock(m_RemovalMutex);
    m_PendingRemovals.clear();
  }

  DirtyComponentSystem<HierarchyComponent>::Shutdown();
}

std::vector<std::type_index> HierarchyComponentSystem::GetSystemDependencies() const
{
  return {typeid(IDComponentSystem)};  // 需要实体ID信息
}

bool HierarchyComponentSystem::OnComponentAdded(ComponentAddedEvent<HierarchyComponent> &e)
{
  Register(&e.GetComponent());

  // 不应当标记事件已处理，继续传播给SceneGraph的HierarchySceneNodeSystem
  // e.Handled();
  return e.handled;
}

bool HierarchyComponentSystem::OnComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e)
{
  auto &oldComponent = e.GetComponent();
  Entity entity = e.GetEntity();

  // 只保存必要的层级关系数据
  Entity parent = oldComponent.GetParent();
  std::vector<Entity> children = oldComponent.GetChildren();  // 拷贝子节点列表

  // 将移除操作加入待处理队列，待下一帧的
  // ProcessDirtyComponents处理待移除的父子关系
  //
  // 分段处理原因：此处无法访问到SceneRegistry，
  // ComponentSystem也不应当维护SceneRegistry对象
  // （该操作复杂度较高，多打LOG方便后续调试）
  {
    std::lock_guard<std::mutex> lock(m_RemovalMutex);
    m_PendingRemovals.emplace_back(entity, parent, children);
  }

  // 从组件列表中移除
  Unregister(&oldComponent);
  m_Logger->debug("Hierarchy component removal queued for entity {}", entity.GetUUIDString());
  m_Logger->trace(
      "Queued removal: parent={}, children_count={}", parent.GetUUIDString(), children.size());

  // 不应当标记事件已处理，继续传播给SceneGraph的HierarchySceneNodeSystem
  // e.Handled();
  return true;
}

void HierarchyComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 1. 首先处理待移除的组件
  ProcessPendingRemovals(registry);

  // 2. 验证并修复所有层级关系的完整性
  ValidateAndRepairHierarchy(registry);

  // 3. 处理脏组件
  for (auto *comp : m_DirtyComponents) {
    comp->ProcessDirty(deltaTime, registry);
    comp->ClearDirty();
  }

  // 4. 清空脏组件列表
  m_DirtyComponents.clear();
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

void HierarchyComponentSystem::ProcessPendingRemovals(SceneRegistry &registry)
{
  std::vector<PendingRemoval> processingRemovals;

  {
    std::lock_guard<std::mutex> lock(m_RemovalMutex);
    if (m_PendingRemovals.empty()) {
      return;
    }

    // 交换数据以减少锁持有时间
    processingRemovals.swap(m_PendingRemovals);
  }
  // 处理所有待移除的组件
  for (const auto &removal : processingRemovals) {
    Entity entity = removal.entity;

    m_Logger->debug("Processing hierarchy component removal for entity {}",
                    entity.GetUUIDString());
    // 1. 从父节点中移除自己（如果父节点存在且有效）
    if (removal.parent.IsValid() && registry.IsValid(removal.parent)) {
      if (registry.HasComponent<HierarchyComponent>(removal.parent)) {
        try {
          auto &parentHierarchy = registry.GetComponent<HierarchyComponent>(removal.parent);
          parentHierarchy.RemoveChild(registry, entity);
          m_Logger->trace("Removed entity {} from parent {}",
                          entity.GetUUIDString(),
                          removal.parent.GetUUIDString());
        }
        catch (const std::exception &e) {
          m_Logger->warn("Failed to remove entity {} from parent {}: {}",
                         entity.GetUUIDString(),
                         removal.parent.GetUUIDString(),
                         e.what());
        }
      }
    }
    // 2. 清除所有子节点的父节点（如果子节点存在且有效）
    for (auto child : removal.children) {
      if (child.IsValid() && registry.IsValid(child)) {
        if (registry.HasComponent<HierarchyComponent>(child)) {
          try {
            auto &childHierarchy = registry.GetComponent<HierarchyComponent>(child);
            // 只有当当前父节点确实是这个被移除的实体时才清除
            if (childHierarchy.GetParent() == entity) {
              childHierarchy.SetParent(registry, Entity());
              m_Logger->trace("Cleared parent for child entity {}", child.GetUUIDString());
            }
          }
          catch (const std::exception &e) {
            m_Logger->warn(
                "Failed to clear parent for child entity {}: {}", child.GetUUIDString(), e.what());
          }
        }
      }
    }
    m_Logger->debug("Completed hierarchy component removal for entity {}", entity.GetUUIDString());
  }
  m_Logger->trace("Processed {} pending hierarchy component removals", processingRemovals.size());
}


};  // namespace mite