#include "scene_graph.h"

namespace mite {
SceneGraph::SceneGraph()
{
  // 订阅实体生命周期事件
  m_EventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));
  m_EventSubscriptions.Subscribe<EntityDestroyedEvent>(BIND_DISPATCH_FN(OnEntityDestroyed));

  // 订阅必要组件构造事件
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentAdded));
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<BoundingVolumeComponent>>(
      BIND_DISPATCH_FN(OnBoundingVolumeComponentAdded));
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
size_t SceneGraph::GetNodeCount() const
{
  return m_NodeManager->GetNodeCount();
}
bool SceneGraph::IsEmpty() const
{
  return m_NodeManager->IsEmpty();
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
    m_NodeManager->MarkNodeDirtyRecursive(entity);
  }
  else {
    m_NodeManager->MarkNodeDirty(entity);
  }
}
void SceneGraph::Update(SceneRegistry &registry)
{
  // 处理等待队列
  ProcessScheduledCreationsAndDestruction(registry);

  // NodeManager->Update()包含了SpatialPartition的Update()
  m_NodeManager->Update(registry);
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
bool SceneGraph::OnEntityCreated(EntityCreatedEvent &event)
{
  Entity entity = event.GetEntity();

  // 将实体加入待创建队列
  if (m_PendingCreateNodes.find(entity) == m_PendingCreateNodes.end()) {
    m_PendingCreateNodes.insert({entity, {false, false}});
  }

  // 阻断事件传播
  event.Handled();
  return true;
}
bool SceneGraph::OnEntityDestroyed(EntityDestroyedEvent &event)
{
  Entity entity = event.GetEntity();

  // 如果该实体有对应的场景节点，则销毁
  if (m_NodeManager->HasNode(entity) &&
      m_PendingDestroyNodes.find(entity) == m_PendingDestroyNodes.end())
  {
    m_PendingDestroyNodes.insert(entity);
  }

  // 不应当阻断事件传播
  return true;
}
bool SceneGraph::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &event)
{
  Entity entity = event.GetEntity();

  // 执行匹配
  if (m_PendingCreateNodes.find(entity) != m_PendingCreateNodes.end()) {
    m_PendingCreateNodes[entity].hasTransform = true;
  }

  // 不应当阻断事件传播
  return true;
}
bool SceneGraph::OnBoundingVolumeComponentAdded(
    ComponentAddedEvent<BoundingVolumeComponent> &event)
{
  Entity entity = event.GetEntity();

  // 执行匹配
  if (m_PendingCreateNodes.find(entity) != m_PendingCreateNodes.end()) {
    m_PendingCreateNodes[entity].hasBoundingVolume = true;
  }

  // 不应当阻断事件传播
  return true;
}

bool SceneGraph::ProcessScheduledCreationsAndDestruction(SceneRegistry &registry)
{
  if (!m_PendingCreateNodes.empty()) {
    return;
  }

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
  for (auto [entity, hasComponents] : m_PendingCreateNodes) {
    if (hasComponents.hasBoundingVolume && hasComponents.hasTransform) {
      // 符合要求,创建Node并将其从待创建队列移除
      CreateNode(registry, entity);
      m_PendingCreateNodes.erase(entity);
    }
  }


}
}  // namespace mite