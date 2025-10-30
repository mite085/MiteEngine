#include "scene_graph.h"
#include "light_core/light_manager.h"

namespace mite {
SceneGraph::SceneGraph()
{
  // 订阅实体生命周期事件 - 高优先级
  // Immediate同步模式：
  // 场景图需要实时响应实体和组件的生命周期变化，确保场景状态一致性
  m_EventSubscriptions.SubscribeImmediate<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated),
                                                              EventPriority::High);
  m_EventSubscriptions.SubscribeImmediate<EntityDestroyedEvent>(
      BIND_DISPATCH_FN(OnEntityDestroyed), EventPriority::High);

  // 订阅必要组件构造事件 - 普通优先级
  // Immediate同步模式：
  // 场景图需要实时响应实体和组件的生命周期变化，确保场景状态一致性
  m_EventSubscriptions.SubscribeImmediate<ComponentAddedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentAdded), EventPriority::Normal);
  m_EventSubscriptions.SubscribeImmediate<ComponentAddedEvent<BoundingVolumeComponent>>(
      BIND_DISPATCH_FN(OnBoundingVolumeComponentAdded), EventPriority::Normal);
}

void SceneGraph::Initialize()
{
  // 按照依赖顺序创建
  m_SpatialPartition = CreateSpatialPartition(SpatialPartitionType::BVH);
  m_NodeManager = std::make_unique<SceneNodeManager>(*m_SpatialPartition);
}

void SceneGraph::CleanUp()
{
  // 按照依赖倒序清理
  m_NodeManager->Clear();
  m_SpatialPartition->Clear();
}

// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneGraph::CreateNode(SceneRegistry &registry, Entity entity)
{
  // NodeManager->CreateNode()包含了SpatialPartition的Insert()
  return m_NodeManager->CreateNode(registry, entity);
}
bool SceneGraph::DestroyNode(SceneRegistry &registry, Entity entity)
{
  // NodeManager->DestroyNode()包含了SpatialPartition的Remove()
  return m_NodeManager->DestroyNode(registry, entity);
}
void SceneGraph::RebuildSpatialPartition()
{
  m_SpatialPartition->Clear();

  // 重新插入所有节点
  auto allNodes = m_NodeManager->GetAllNodes();
  for (SceneNode *node : allNodes) {
    m_SpatialPartition->Insert(node);
  }
}
// ==================== 场景节点查询接口 ====================
SceneNode *SceneGraph::GetNode(Entity entity) const
{
  return m_NodeManager->GetNode(entity);
}
bool SceneGraph::HasNode(Entity entity) const
{
  return m_NodeManager->HasNode(entity);
}
SceneNode *SceneGraph::FindNodeByPath(const std::string &path) const
{
  return m_NodeManager->FindNodeByPath(path);
}
std::vector<SceneNode *> SceneGraph::GetRootNodes() const
{
  return m_NodeManager->GetRootNodes();
}
std::vector<SceneNode *> SceneGraph::GetAllNodes() const
{
  return m_NodeManager->GetAllNodes();
}

// ==================== 空间查询接口 ====================
std::vector<SceneNode *> SceneGraph::FrustumCull(const Frustum &frustum,
                                                 const uint32_t visibleMask) const
{
  std::vector<SceneNode *> results;
  m_SpatialPartition->FrustumCull(frustum, visibleMask, results);
  return results;
}
std::vector<SceneNode *> SceneGraph::Raycast(const Ray &ray) const
{
  std::vector<SceneNode *> results;
  m_SpatialPartition->Raycast(ray, results);
  return results;
}
SceneNode *SceneGraph::RaycastFirst(const Ray &ray) const
{
  SceneNode *result;
  float distance;
  m_SpatialPartition->RaycastFirst(ray, result, distance);
  return result;
}
std::vector<SceneNode *> SceneGraph::VolumeQuery(const BoundingVolume &volume) const
{
  std::vector<SceneNode *> results;
  m_SpatialPartition->VolumeQuery(volume, results);
  return results;
}
std::vector<SceneNode *> SceneGraph::PointQuery(const glm::vec3 &point) const
{
  std::vector<SceneNode *> results;
  m_SpatialPartition->PointQuery(point, results);
  return results;
}
// ==================== 场景图遍历接口 ====================
void SceneGraph::Traverse(std::function<bool(SceneNode *)> callback,
                          SceneNodeManager::TraversalType type) const
{
  m_NodeManager->TraverseTree(callback, type);
}
void SceneGraph::TraverseVisible(std::function<bool(SceneNode *)> callback,
                                 SceneNodeManager::TraversalType type) const
{
  // callback的再包装，添加WorldVisible判断
  auto visibleFilter = [callback](SceneNode *node) {
    if (node->IsWorldVisible()) {
      return callback(node);
    }
    return true;  // 继续遍历
  };

  Traverse(visibleFilter, type);
}

// ==================== 更新管理接口 ====================
void SceneGraph::MarkDirty(Entity entity, bool recursive)
{
  if (recursive) {
    m_NodeManager->MarkNodeDirtyRecursive(GetNode(entity));
  }
  else {
    m_NodeManager->MarkNodeDirty(GetNode(entity));
  }
}
void SceneGraph::Update(SceneRegistry &registry)
{
  // 处理等待队列
  ProcessScheduledCreationsAndDestruction(registry);

  // NodeManager->Update()包含了SpatialPartition的Update()
  m_NodeManager->Update(registry);

  // 根据光照数据和新的世界变换组建光源数据
  std::unordered_map<std::shared_ptr<Light>, Transform> lightTransforms;
  for (SceneNode *node : m_NodeManager->GetLightNodes()) {
    if (node && registry.HasComponent<LightComponent>(node->GetEntity())) {
      // 获取组件光源数据
      std::shared_ptr<Light> light =
          registry.GetComponent<LightComponent>(node->GetEntity()).GetLight();
      lightTransforms.insert(std::make_pair(light, node->GetWorldTransform()));
    }
  }

  // 使用光源数据更新LightData
  LightManager::Get().UpdateLightData(lightTransforms);
}

// ==================== 状态查询接口 ====================
size_t SceneGraph::GetNodeCount() const
{
  return m_NodeManager->GetNodeCount();
}
bool SceneGraph::IsEmpty() const
{
  return m_NodeManager->IsEmpty();
}
std::string SceneGraph::GetStats() const
{
  std::stringstream ss;
  ss << "SceneGraph Stats:\n";
  ss << "  Total Nodes: " << GetNodeCount() << "\n";
  ss << "  Spatial Partition: " << m_SpatialPartition->GetTypeName() << "\n";
  ss << "  Partition Depth: " << m_SpatialPartition->GetDepth() << "\n";
  ss << m_SpatialPartition->GetStats();
  return ss.str();
}

// ==================== 调试接口 ====================
void SceneGraph::DebugDraw(
    std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback) const
{
  m_SpatialPartition->DebugDraw(drawCallback);
}

// ==================== 事件响应 ====================
void SceneGraph::OnEntityCreated(EntityCreatedEvent &event)
{
  Entity entity = event.GetEntity();

  // 将实体加入待创建队列
  if (m_PendingCreateNodes.find(entity) == m_PendingCreateNodes.end()) {
    m_PendingCreateNodes.insert({entity, {false, false}});
  }

  // 标记已处理但允许传播（其他系统可能需要知道实体创建）
  event.SetResult(EventResult::Handled);
}
void SceneGraph::OnEntityDestroyed(EntityDestroyedEvent &event)
{
  Entity entity = event.GetEntity();

  // 如果该实体有对应的场景节点，则销毁
  if (m_NodeManager->HasNode(entity) &&
      m_PendingDestroyNodes.find(entity) == m_PendingDestroyNodes.end())
  {
    m_PendingDestroyNodes.insert(entity);
  }

  // 不阻断事件传播（其他系统需要知道实体销毁）
  event.SetResult(EventResult::None);
}
void SceneGraph::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &event)
{
  Entity entity = event.GetEntity();

  // 执行匹配
  if (m_PendingCreateNodes.find(entity) != m_PendingCreateNodes.end()) {
    m_PendingCreateNodes[entity].hasTransform = true;
  }

  // 不阻断事件传播（其他系统可能需要Transform组件）
  event.SetResult(EventResult::None);
}
void SceneGraph::OnBoundingVolumeComponentAdded(
    ComponentAddedEvent<BoundingVolumeComponent> &event)
{
  Entity entity = event.GetEntity();

  // 执行匹配
  if (m_PendingCreateNodes.find(entity) != m_PendingCreateNodes.end()) {
    m_PendingCreateNodes[entity].hasBoundingVolume = true;
  }

  // 不阻断事件传播（其他系统可能需要BoundingVolume组件）
  event.SetResult(EventResult::None);
}

void SceneGraph::ProcessScheduledCreationsAndDestruction(SceneRegistry &registry)
{
  // 1. 遍历待删除的实体队列(该步骤可能修改待创建队列,要先处理)
  for (auto entity : m_PendingDestroyNodes) {
    // 若存在于SceneNodeManager,正常销毁
    if (m_NodeManager->HasNode(entity))
      DestroyNode(registry, entity);

    // 若存在于待添加队列,一并移除
    if (m_PendingCreateNodes.find(entity) != m_PendingCreateNodes.end())
      m_PendingCreateNodes.erase(entity);

    // 待销毁队列仅维护一次,不管是否销毁都不再尝试,除非再次加入待销毁队列
    m_PendingCreateNodes.erase(entity);
  }

  // 2. 遍历待创建的实体队列
  std::vector<Entity> toRemove;
  for (auto [entity, hasComponents] : m_PendingCreateNodes) {
    if (hasComponents.hasBoundingVolume && hasComponents.hasTransform) {
      // 符合要求,创建Node并记录进待删除列表
      CreateNode(registry, entity);
      toRemove.push_back(entity);
    }
  }
  // 遍历结束后统一删除
  for (auto entity : toRemove) {
    m_PendingCreateNodes.erase(entity);
  }
}
}  // namespace mite