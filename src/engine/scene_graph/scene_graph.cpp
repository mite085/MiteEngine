#include "scene_graph.h"
#include "simple_bvh.h"

namespace mite {

// ==================== 构造函数和析构函数 ====================
SceneGraph::SceneGraph(SpatialPartitionType spatialPartitionType)
    : m_spatialPartitionType(spatialPartitionType)
{
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph");
  m_logger->trace("SceneGraph created with spatial partition type: {}",
                  GetSpatialPartitionTypeName(m_spatialPartitionType));

  // 初始化空间划分结构
  InitializeSpatialPartition();
}

SceneGraph::~SceneGraph()
{
  m_logger->info("Destroying SceneGraph");
  Clear();
  m_logger->debug("SceneGraph destroyed");
}

// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneGraph::CreateNode(Entity entity)
{
  if (!entity.IsValid()) {
    m_logger->warn("Attempted to create node for invalid entity");
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  // 检查是否已存在节点
  if (m_entityToNodeMap.find(entity) != m_entityToNodeMap.end()) {
    m_logger->warn("Scene node already exists for entity {}", entity.GetUUIDString());
    return m_entityToNodeMap[entity].get();
  }

  try {
    // 创建新的场景节点
    auto node = std::make_unique<SceneNode>(entity);
    SceneNode *nodePtr = node.get();

    // 添加到映射表
    m_entityToNodeMap[entity] = std::move(node);

    // 添加到空间划分结构
    AddNodeToSpatialPartition(nodePtr);

    m_logger->debug("Created scene node for entity {}", entity.GetUUIDString());
    return nodePtr;
  }
  catch (const std::exception &e) {
    m_logger->error(
        "Failed to create scene node for entity {}: {}", entity.GetUUIDString(), e.what());
    return nullptr;
  }
}

bool SceneGraph::DestroyNode(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_entityToNodeMap.find(entity);
  if (it == m_entityToNodeMap.end()) {
    m_logger->warn("Scene node not found for entity {}", entity.GetUUIDString());
    return false;
  }

  SceneNode *node = it->second.get();

  // 从空间划分结构中移除
  RemoveNodeFromSpatialPartition(node);

  // 处理父子关系：将所有子节点提升为根节点
  auto children = node->GetChildren();
  for (SceneNode *child : children) {
    SetParent(child, nullptr);
  }

  // 如果自身有父节点，从父节点中移除
  if (node->GetParent()) {
    node->GetParent()->RemoveChild(node);
  }

  // 从映射表中移除
  m_entityToNodeMap.erase(it);

  // 从脏节点列表中移除
  m_dirtyNodes.erase(std::remove(m_dirtyNodes.begin(), m_dirtyNodes.end(), entity),
                     m_dirtyNodes.end());

  m_logger->debug("Destroyed scene node for entity {}", entity.GetUUIDString());
  return true;
}

void SceneGraph::CreateNodes(const std::vector<Entity> &entities)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (Entity entity : entities) {
    if (entity.IsValid() && m_entityToNodeMap.find(entity) == m_entityToNodeMap.end()) {
      CreateNode(entity);
    }
  }

  m_logger->debug("Created {} scene nodes in batch", entities.size());
}

void SceneGraph::DestroyNodes(const std::vector<Entity> &entities)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (Entity entity : entities) {
    DestroyNode(entity);
  }

  m_logger->debug("Destroyed {} scene nodes in batch", entities.size());
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneGraph::GetNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_entityToNodeMap.find(entity);
  return it != m_entityToNodeMap.end() ? it->second.get() : nullptr;
}

bool SceneGraph::HasNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.find(entity) != m_entityToNodeMap.end();
}

std::vector<SceneNode *> SceneGraph::GetRootNodes() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> rootNodes;

  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (node->IsRoot()) {
      rootNodes.push_back(node.get());
    }
  }

  return rootNodes;
}

std::vector<SceneNode *> SceneGraph::GetAllNodes() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> nodes;
  nodes.reserve(m_entityToNodeMap.size());

  for (const auto &[entity, node] : m_entityToNodeMap) {
    nodes.push_back(node.get());
  }

  return nodes;
}

size_t SceneGraph::GetNodeCount() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.size();
}

bool SceneGraph::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.empty();
}

// ==================== 场景树操作接口（编辑器支持） ====================
bool SceneGraph::SetParent(SceneNode *node, SceneNode *newParent)
{
  if (!node) {
    m_logger->warn("Attempted to set parent for null node");
    return false;
  }

  // 检查循环引用
  if (!ValidateParenting(node, newParent)) {
    m_logger->warn("Invalid parenting operation: cyclic reference detected");
    return false;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  // 从原父节点移除
  SceneNode *oldParent = node->GetParent();
  if (oldParent) {
    oldParent->RemoveChild(node);
  }

  // 设置新父节点
  if (newParent) {
    newParent->AddChild(node);
  }

  // 设置节点的父节点引用
  node->SetParent(newParent);

  // 标记节点需要更新（父子关系变化影响世界变换）
  MarkNodeDirty(node->GetEntity());

  m_logger->debug("Reparented node {}.", node->GetEntity().GetUUIDString());

  return true;
}

std::string SceneGraph::GetNodePath(SceneNode *node) const
{
  if (!node) {
    return "Invalid";
  }

  std::vector<std::string> pathSegments;
  SceneNode *current = node;

  // 向上遍历构建路径
  while (current) {
    std::stringstream ss;
    ss << "Entity_" << current->GetEntity().GetUUIDString();
    pathSegments.push_back(ss.str());
    current = current->GetParent();
  }

  // 反转路径（从根到当前节点）
  std::reverse(pathSegments.begin(), pathSegments.end());

  // 拼接路径字符串
  std::string path;
  for (size_t i = 0; i < pathSegments.size(); ++i) {
    if (i > 0)
      path += "/";
    path += pathSegments[i];
  }

  return path;
}

SceneNode *SceneGraph::FindNodeByPath(const std::string &path) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 简单的路径查找实现（可根据需要优化）
  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (GetNodePath(node.get()) == path) {
      return node.get();
    }
  }

  return nullptr;
}

void SceneGraph::TraverseTree(std::function<bool(SceneNode *)> callback) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 从所有根节点开始遍历
  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (node->IsRoot()) {
      if (!TraverseRecursive(node.get(), callback)) {
        break;  // 回调函数要求中断遍历
      }
    }
  }
}

// ==================== 空间划分管理接口 ====================
void SceneGraph::SetSpatialPartitionType(SpatialPartitionType type)
{
  if (m_spatialPartitionType == type) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  m_spatialPartitionType = type;
  InitializeSpatialPartition();

  m_logger->info("Spatial partition type changed to: {}", GetSpatialPartitionTypeName(type));
}

SpatialPartitionType SceneGraph::GetSpatialPartitionType() const
{
  return m_spatialPartitionType;
}

void SceneGraph::RebuildSpatialPartition()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Cannot rebuild null spatial partition");
    return;
  }

  m_spatialPartition->Clear();

  // 重新添加所有节点
  for (const auto &[entity, node] : m_entityToNodeMap) {
    AddNodeToSpatialPartition(node.get());
  }

  m_logger->debug("Rebuilt spatial partition with {} nodes", m_entityToNodeMap.size());
}

std::string SceneGraph::GetSpatialPartitionStats() const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    return "Spatial partition not initialized";
  }

  return m_spatialPartition->GetStats();
}

void SceneGraph::DebugDraw(std::function<void(const AABB &, int depth)> drawCallback)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_spatialPartition && drawCallback) {
    m_spatialPartition->DebugDraw(drawCallback);
  }
}

// ==================== 空间查询接口 ====================
int SceneGraph::QueryVisibleNodes(const Frustum &frustum, std::vector<SceneNode *> &results)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Spatial partition not initialized for frustum culling");
    return 0;
  }

  results.clear();
  return m_spatialPartition->FrustumCull(frustum, results);
}

size_t SceneGraph::QueryRaycast(const Ray &ray, std::vector<SceneNode *> &results)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Spatial partition not initialized for raycast");
    return 0;
  }

  results.clear();
  if (m_spatialPartition->Raycast(ray, results)) {
    return results.size();
  }
  return 0;
}

bool SceneGraph::QueryRaycastFirst(const Ray &ray, SceneNode *&result, float &distance)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Spatial partition not initialized for raycast");
    return false;
  }

  return m_spatialPartition->RaycastFirst(ray, result, distance);
}

int SceneGraph::QuerySphere(const Sphere &sphere, std::vector<SceneNode *> &results)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Spatial partition not initialized for sphere query");
    return 0;
  }

  results.clear();
  return m_spatialPartition->SphereQuery(sphere, results);
}

int SceneGraph::QueryAABB(const AABB &aabb, std::vector<SceneNode *> &results)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Spatial partition not initialized for AABB query");
    return 0;
  }

  results.clear();
  return m_spatialPartition->AABBQuery(aabb, results);
}

// ==================== 节点更新接口 ====================
void SceneGraph::UpdateNodeTransform(Entity entity, const glm::mat4 &localTransform)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_entityToNodeMap.find(entity);
  if (it != m_entityToNodeMap.end()) {
    it->second->SetLocalTransform(localTransform);
    MarkNodeDirty(entity);
  }
}

void SceneGraph::UpdateNodeBounds(Entity entity, const AABB &localBounds)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_entityToNodeMap.find(entity);
  if (it != m_entityToNodeMap.end()) {
    it->second->SetLocalBounds(localBounds);
    MarkNodeDirty(entity);
  }
}

void SceneGraph::MarkNodeDirty(Entity entity)
{
  // 避免重复添加
  if (std::find(m_dirtyNodes.begin(), m_dirtyNodes.end(), entity) == m_dirtyNodes.end()) {
    m_dirtyNodes.push_back(entity);
  }
}

void SceneGraph::UpdateDirtyNodes()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_dirtyNodes.empty()) {
    return;
  }

  // 更新所有脏节点
  for (Entity entity : m_dirtyNodes) {
    auto it = m_entityToNodeMap.find(entity);
    if (it != m_entityToNodeMap.end()) {
      it->second->Update();

      // 更新空间划分结构中的节点位置
      if (m_spatialPartition) {
        m_spatialPartition->Update(it->second.get());
      }
    }
  }

  m_logger->trace("Updated {} dirty nodes", m_dirtyNodes.size());
  m_dirtyNodes.clear();
}

// ==================== 序列化支持 ====================
bool SceneGraph::Serialize(std::ostream &output) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // TODO: 实现完整的场景图序列化
  // 目前先预留接口
  m_logger->info("SceneGraph serialization called (not implemented)");
  return !output.fail();
}

bool SceneGraph::Deserialize(std::istream &input)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // TODO: 实现完整的场景图反序列化
  // 目前先预留接口
  m_logger->info("SceneGraph deserialization called (not implemented)");
  return !input.fail();
}

// ==================== 私有工具方法 ====================
void SceneGraph::InitializeSpatialPartition()
{
  m_spatialPartition = CreateSpatialPartition(m_spatialPartitionType);
  if (m_spatialPartition) {
    m_logger->debug("Initialized {} spatial partition",
                    GetSpatialPartitionTypeName(m_spatialPartitionType));
  }
  else {
    m_logger->error("Failed to initialize spatial partition");
  }
}

bool SceneGraph::TraverseRecursive(SceneNode *node,
                                   std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }

  // 先处理当前节点
  if (!callback(node)) {
    return false;  // 回调要求中断遍历
  }

  // 递归处理所有子节点
  for (SceneNode *child : node->GetChildren()) {
    if (!TraverseRecursive(child, callback)) {
      return false;
    }
  }

  return true;
}

bool SceneGraph::ValidateParenting(SceneNode *node, SceneNode *newParent) const
{
  if (!node || node == newParent) {
    return false;  // 不能设置自己为父节点
  }

  // 检查循环引用：确保newParent不是node的子孙
  SceneNode *current = newParent;
  while (current) {
    if (current == node) {
      return false;  // 循环引用检测
    }
    current = current->GetParent();
  }

  return true;
}

void SceneGraph::RemoveNodeFromSpatialPartition(SceneNode *node)
{
  if (m_spatialPartition && node) {
    m_spatialPartition->Remove(node);
  }
}

void SceneGraph::AddNodeToSpatialPartition(SceneNode *node)
{
  if (m_spatialPartition && node) {
    m_spatialPartition->Insert(node);
  }
}

void SceneGraph::Clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 清空空间划分结构
  if (m_spatialPartition) {
    m_spatialPartition->Clear();
  }

  // 清空所有节点（会自动处理父子关系）
  m_entityToNodeMap.clear();
  m_dirtyNodes.clear();

  m_logger->debug("SceneGraph cleared");
}

}  // namespace mite
