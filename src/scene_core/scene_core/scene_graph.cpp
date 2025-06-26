#include "scene_graph.h"


namespace mite {
SceneGraph::SceneGraph()
{  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Scene Graph");
  m_Logger->trace("Created scene graph");
}
SceneGraph::~SceneGraph()
{
  Clear();
}
void SceneGraph::Initialize(SceneRegistry &registry)
{
  m_Registry = registry;

  // 订阅相关事件
  m_EventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));
  m_EventSubscriptions.Subscribe<EntityDestroyedEvent>(BIND_DISPATCH_FN(OnEntityDestroyed));
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyAdded));
  m_EventSubscriptions.Subscribe<ComponentChangedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyChanged));
  m_EventSubscriptions.Subscribe<ComponentRemovedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyRemoved));
  m_EventSubscriptions.Subscribe<TransformUpdatedEvent>(BIND_DISPATCH_FN(OnTransformChanged));
  m_EventSubscriptions.Subscribe<PositionChangedEvent>(BIND_DISPATCH_FN(OnPositionChanged));
  m_EventSubscriptions.Subscribe<RotationChangedEvent>(BIND_DISPATCH_FN(OnRotationChanged));
  m_EventSubscriptions.Subscribe<ScaleChangedEvent>(BIND_DISPATCH_FN(OnScaleChanged));
  m_EventSubscriptions.Subscribe<TransformChangedEvent>(BIND_DISPATCH_FN(OnTransformChanged));
}

void SceneGraph::Clear()
{
  m_EventSubscriptions.UnsubscribeAll();
}

void SceneGraph::OnUpdate(float timestep)
{
  // 1. 处理变换继承和可见性继承（使用事件系统和ComponentSystem系统的每帧更新替代） --------------------------------

  // 2. 处理延迟的层次结构变更 ----------------------------------

  //// TODO: 如果有延迟的父子关系变更，在这里处理
  // ProcessDeferredHierarchyChanges();

  // 3. 更新场景图统计信息 --------------------------------------

  //// TODO: 按照时间step更新统计信息
  // UpdateSceneStatistics(timestep);
}
bool SceneGraph::SetParent(Entity entity, Entity newParent)
{
  // 检查实体有效性
  if (!GetRegistry().IsValid(entity)) {
    return false;
  }

  // 检查parent有效性 (疑问：空实体是否能成为合法的parent?)
  if (!GetRegistry().IsValid(newParent)) {
    return false;
  }

  // 检查是否设置为自己父节点
  if (entity == newParent) {
    return false;
  }

  // 检查是否形成循环依赖
  if (newParent.IsValid() && ValidateHierarchy(entity, newParent)) {
    return false;
  }

  // 获取当前父节点
  auto &hierarchy = GetRegistry().GetOrAddComponent<HierarchyComponent>(entity);
  Entity oldParent = hierarchy.GetParent();

  // 如果父节点没有变化，直接返回成功
  if (oldParent == newParent) {
    return true;
  }

  // 从旧父节点中移除
  if (oldParent.IsValid()) {
    auto &oldParentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(oldParent);
    oldParentHierarchy.RemoveChild(entity);
  }

  // 设置新父节点
  hierarchy.SetParent(newParent);

  // 添加到新父节点的子列表
  if (newParent.IsValid()) {
    auto &newParentHierarchy = GetRegistry().GetOrAddComponent<HierarchyComponent>(newParent);
    newParentHierarchy.AddChild(entity);
  }

  // 重新计算受影响实体的深度
  // 这里可以优化为只更新子树的深度
  RecalculateAllDepths();

  return true;
}

Entity SceneGraph::GetParent(Entity entity) const
{
  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    return hierarchy->GetParent();
  }
  return Entity();
}

const std::vector<Entity> &SceneGraph::GetChildren(Entity entity) const
{
  static const std::vector<Entity> emptyChildren;

  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    return hierarchy->GetChildren();
  }
  return emptyChildren;
}

bool SceneGraph::IsRoot(Entity entity) const
{
  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    return hierarchy->IsRoot();
  }
  return true;  // 没有HierarchyComponent的实体视为根节点
}

bool SceneGraph::IsLeaf(Entity entity) const
{
  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    return hierarchy->IsLeaf();
  }
  return true;  // 没有HierarchyComponent的实体视为叶节点
}

size_t SceneGraph::GetDepth(Entity entity) 
{
  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    return hierarchy->GetDepth(GetRegistry());
  }
  return 0;  // 没有HierarchyComponent的实体深度为0
}

void SceneGraph::Traverse(Entity root, const VisitorFunc &visitor, TraversalOrder order) const
{
  if (!GetRegistry().IsValid(root) || !visitor) {
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

void SceneGraph::TraverseAll(const VisitorFunc &visitor, TraversalOrder order)
{
  auto roots = GetRoots();
  for (auto root : roots) {
    Traverse(root, visitor, order);
  }
}

std::vector<Entity> SceneGraph::GetPathToRoot(Entity entity) const
{
  std::vector<Entity> path;

  while (GetRegistry().IsValid(entity)) {
    path.push_back(entity);
    entity = GetParent(entity);
  }

  return path;
}

bool SceneGraph::IsInSameHierarchy(Entity entity1, Entity entity2) const
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

std::vector<Entity> SceneGraph::GetRoots()
{
  std::vector<Entity> roots;

  // 遍历所有有HierarchyComponent的实体，收集根节点
  auto &storage = GetRegistry().GetAllEntities();
  for (auto entity : storage) {
    if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
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
  std::queue<std::pair<Entity, size_t>> queue;

  // 初始队列包含所有根节点，深度为0
  auto roots = GetRoots();
  for (auto root : roots) {
    queue.emplace(root, 0);
  }

  while (!queue.empty()) {
    auto [entity, depth] = queue.front();
    queue.pop();

    if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
      hierarchy->m_DepthCache = depth;

      // 将子节点加入队列，深度+1
      for (auto child : hierarchy->GetChildren()) {
        queue.emplace(child, depth + 1);
      }
    }
  }
}

void SceneGraph::OnRenderPrepare()
{
}

bool SceneGraph::TraverseDFS(Entity entity, const VisitorFunc &visitor) const
{
  if (!visitor(entity)) {
    return false;
  }

  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    for (auto child : hierarchy->GetChildren()) {
      if (!TraverseDFS(child, visitor)) {
        return false;
      }
    }
  }

  return true;
}

void SceneGraph::TraverseBFS(Entity entity, const VisitorFunc &visitor) const
{
  std::queue<Entity> queue;
  queue.push(entity);

  while (!queue.empty()) {
    auto current = queue.front();
    queue.pop();

    if (!visitor(current)) {
      return;
    }

    if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(current)) {
      for (auto child : hierarchy->GetChildren()) {
        queue.push(child);
      }
    }
  }
}

bool SceneGraph::TraverseReverseDFS(Entity entity, const VisitorFunc &visitor) const
{
  if (auto *hierarchy = GetRegistry().TryGetComponent<HierarchyComponent>(entity)) {
    for (auto child : hierarchy->GetChildren()) {
      if (!TraverseReverseDFS(child, visitor)) {
        return false;
      }
    }
  }

  return visitor(entity);
}

// 事件处理实现 ==========================================

void SceneGraph::OnEntityCreated(EntityCreatedEvent &e)
{
  Entity entity = e.GetEntity();

  // 为新实体添加默认层次组件（如果不存在）
  if (!GetRegistry().HasComponent<HierarchyComponent>(entity)) {
    GetRegistry().AddComponent<HierarchyComponent>(entity);
  }
}

void SceneGraph::OnEntityDestroyed(EntityDestroyedEvent &e)
{
  Entity entity = e.GetEntity();

  // 如果实体有层次组件，需要清理父子关系
  if (GetRegistry().HasComponent<HierarchyComponent>(entity)) {
    auto &hierarchy = GetRegistry().GetComponent<HierarchyComponent>(entity);

    // 1. 从父节点移除自己
    if (hierarchy.GetParent().IsValid()) {
      if (GetRegistry().HasComponent<HierarchyComponent>(hierarchy.GetParent())) {
        auto &parentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
            hierarchy.GetParent());
        parentHierarchy.RemoveChild(entity);
      }
    }

    // 2. 将所有子节点提升为根节点
    for (Entity child : hierarchy.GetChildren()) {
      if (GetRegistry().IsValid(child) && GetRegistry().HasComponent<HierarchyComponent>(child)) {
        auto &childHierarchy = GetRegistry().GetComponent<HierarchyComponent>(child);
        childHierarchy.SetParent(Entity());

        // 标记子节点变换为脏，需要重新计算世界变换
        if (GetRegistry().HasComponent<TransformComponent>(child)) {
          auto &transform = GetRegistry().GetComponent<TransformComponent>(child);
          transform.dirtyFlags |= TransformComponent::HIERARCHY_DIRTY;
          transform.MarkDirty();
        }
      }
    }
  }
}

void SceneGraph::OnHierarchyAdded(ComponentAddedEvent<HierarchyComponent> &e)
{
  Entity entity = e.GetEntity();
  auto &hierarchy = e.GetComponent();

  // 新添加的层次组件需要初始化
  hierarchy.SetParent(Entity());  // 默认无父节点

  // 可以在这里添加默认子节点或执行其他初始化逻辑
}

void SceneGraph::OnHierarchyChanged(ComponentChangedEvent<HierarchyComponent> &e)
{
  Entity entity = e.GetEntity();
  auto &newHierarchy = e.GetComponent();
  auto& oldHierarchy = e.GetOldComponent(); // 需要适配器支持获取旧组件

  // 1. 验证新父子关系是否有效
  if (!ValidateHierarchy(entity, newHierarchy.GetParent())) {
    // 如果无效，恢复原来的父节点
     newHierarchy.SetParent(oldHierarchy.GetParent());
    return;
  }

  // 2. 从旧父节点移除当前实体
  if (oldHierarchy.GetParent().IsValid()) {
    if (GetRegistry().HasComponent<HierarchyComponent>(oldHierarchy.GetParent())) {
      auto &oldParentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
          oldHierarchy.GetParent());
      oldParentHierarchy.RemoveChild(entity);
    }
  }

  // 3. 添加到新父节点
  if (newHierarchy.GetParent().IsValid()) {
    if (GetRegistry().HasComponent<HierarchyComponent>(newHierarchy.GetParent())) {
      auto &newParentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
          newHierarchy.GetParent());
      newParentHierarchy.AddChild(entity);
    }
  }

  // 4. 更新深度缓存
  UpdateDepthCacheRecursive(entity);

  // 5. 标记变换为脏，需要重新计算世界变换
  if (GetRegistry().HasComponent<TransformComponent>(entity)) {
    auto &transform = GetRegistry().GetComponent<TransformComponent>(entity);
    transform.dirtyFlags |= TransformComponent::HIERARCHY_DIRTY;
    transform.MarkDirty();
  }

  // 6. 发布层次变更事件
  EventBus::Get().Post(EntityParentChangedEvent(entity));
}

void SceneGraph::OnHierarchyRemoved(ComponentRemovedEvent<HierarchyComponent> &e)
{
  Entity entity = e.GetEntity();
  auto &hierarchy = e.GetComponent();

  // 1. 从父节点移除自己
  if (hierarchy.GetParent().IsValid()) {
    if (GetRegistry().HasComponent<HierarchyComponent>(hierarchy.GetParent())) {
      auto &parentHierarchy = GetRegistry().GetComponent<HierarchyComponent>(
          hierarchy.GetParent());
      parentHierarchy.RemoveChild(entity);
    }
  }

  // 2. 将所有子节点提升为根节点
  for (Entity child : hierarchy.GetChildren()) {
    if (GetRegistry().IsValid(child) && GetRegistry().HasComponent<HierarchyComponent>(child)) {
      auto &childHierarchy = GetRegistry().GetComponent<HierarchyComponent>(child);
      childHierarchy.SetParent(Entity());

      // 标记子节点变换为脏
      if (GetRegistry().HasComponent<TransformComponent>(child)) {
        auto &transform = GetRegistry().GetComponent<TransformComponent>(child);
        transform.dirtyFlags |= TransformComponent::HIERARCHY_DIRTY;
        transform.MarkDirty();
      }
    }
  }
}

void SceneGraph::UpdateDepthCacheRecursive(Entity entity)
{
  if (!entity.IsValid() || !GetRegistry().IsValid(entity)) {
    return;
  }

  if (GetRegistry().HasComponent<HierarchyComponent>(entity)) {
    auto &hierarchy = GetRegistry().GetComponent<HierarchyComponent>(entity);

    // 使当前实体的深度缓存失效
    hierarchy.m_DepthCache = 0;

    // 递归处理所有子实体
    for (Entity child : hierarchy.GetChildren()) {
      UpdateDepthCacheRecursive(child);
    }
  }
}

bool SceneGraph::ValidateHierarchy(Entity child, Entity newParent) const
{
  // 不允许设置自己为自己的父节点
  if (child == newParent) {
    return false;
  }

  // 检查循环依赖
  Entity current = newParent;
  while (current.IsValid() && GetRegistry().IsValid(current)) {
    if (current == child) {
      return false;  // 检测到循环
    }

    if (GetRegistry().HasComponent<HierarchyComponent>(current)) {
      auto &hierarchy = GetRegistry().GetComponent<HierarchyComponent>(current);
      current = hierarchy.GetParent();
    }
    else {
      break;
    }
  }

  return true;
}

void SceneGraph::OnTransformChanged(TransformUpdatedEvent &e)
{
  Entity entity = e.GetEntity();

  // 标记子实体需要更新层次变换
  MarkChildrenDirty(entity, TransformComponent::HIERARCHY_DIRTY);
}

void SceneGraph::OnPositionChanged(PositionChangedEvent &e)
{
  Entity entity = e.GetEntity();

  // 只处理局部空间变更（世界空间变更已在TransformComponent中转换为局部空间）
  if (!e.IsWorldSpace()) {
    // 标记子实体需要更新层次变换
    MarkChildrenDirty(entity, TransformComponent::HIERARCHY_DIRTY);

    // 可以在这里添加空间加速结构更新等逻辑
  }
}

void SceneGraph::OnRotationChanged(RotationChangedEvent &e)
{
  Entity entity = e.GetEntity();

  if (!e.IsWorldSpace()) {
    MarkChildrenDirty(entity, TransformComponent::HIERARCHY_DIRTY);

    // 旋转变更通常需要更新方向相关系统
    // 如：光源方向、摄像机朝向等
  }
}

void SceneGraph::OnScaleChanged(ScaleChangedEvent &e)
{
  Entity entity = e.GetEntity();

  if (!e.IsWorldSpace()) {
    MarkChildrenDirty(entity, TransformComponent::HIERARCHY_DIRTY);

    // 缩放变更可能影响碰撞体、渲染LOD等
  }
}

void SceneGraph::OnTransformChanged(TransformChangedEvent &e)
{
  Entity entity = e.GetEntity();

  if (!e.IsWorldSpace()) {
    MarkChildrenDirty(entity, TransformComponent::HIERARCHY_DIRTY);

    // 完整变换更新通常需要更多系统响应
    // 如：物理系统、动画系统等
  }
}

void SceneGraph::MarkChildrenDirty(Entity entity, uint8_t flags)
{
  if (!GetRegistry().IsValid(entity) || !GetRegistry().HasComponent<HierarchyComponent>(entity)) {
    return;
  }

  auto &hierarchy = GetRegistry().GetComponent<HierarchyComponent>(entity);
  for (Entity child : hierarchy.GetChildren()) {
    if (GetRegistry().IsValid(child) && GetRegistry().HasComponent<TransformComponent>(child)) {
      auto &childTransform = GetRegistry().GetComponent<TransformComponent>(child);
      childTransform.dirtyFlags |= flags;
      childTransform.MarkDirty();

      // 递归处理子节点的子节点
      MarkChildrenDirty(child, flags);
    }
  }
}



};  // namespace mite