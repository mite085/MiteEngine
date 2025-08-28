#include "scene_graph.h"
#include "Hierarchy_scene_node_system.h"
#include "scene_core/component_system_manager.h"
#include "scene_graph_system.h"
#include "simple_bvh.h"
#include "transform_scene_node_system.h"
#include "visibility_component.h"

namespace mite {
// ==================== 构造函数和析构函数 ====================
SceneGraph::SceneGraph(SpatialPartitionType spatialPartitionType)
    : m_spatialPartitionType(spatialPartitionType),
      m_visibleNodeCount(0)
{
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph");
}

SceneGraph::~SceneGraph() {}

void SceneGraph::Initialize(ComponentSystemManager &manager)
{
  manager.RegisterSystem<SceneGraphSystem>();
  manager.RegisterSystem<HierarchySceneNodeSystem>();
  manager.RegisterSystem<TransformSceneNodeSystem>();
  manager.RegisterSystem<VisibilityComponentSystem>();

  // 将创建好的SceneGraph交付给注册在SceneCore模块的SceneGraph组件系统
  manager.GetSystem<SceneGraphSystem>()->SetSceneGraph(this);
  manager.GetSystem<HierarchySceneNodeSystem>()->SetSceneGraph(this);
  manager.GetSystem<TransformSceneNodeSystem>()->SetSceneGraph(this);

  // 初始化空间划分结构（默认BVH）
  InitializeSpatialPartition();
  m_logger->trace("SceneGraph created with spatial partition type: {}",
                  GetSpatialPartitionTypeName(m_spatialPartitionType));
}

void SceneGraph::CleanUp(ComponentSystemManager &manager)
{
  m_logger->info("Destroying SceneGraph");

  Clear();
  manager.UnregisterSystem<SceneGraphSystem>();
  manager.UnregisterSystem<HierarchySceneNodeSystem>();
  manager.UnregisterSystem<TransformSceneNodeSystem>();
  manager.UnregisterSystem<VisibilityComponentSystem>();

  m_logger->debug("SceneGraph destroyed");
}

// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneGraph::CreateNode(SceneRegistry &registry, Entity entity)
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

    // 如果实体有VisibilityComponent，初始化其局部包围盒
    if (registry.HasComponent<VisibilityComponent>(entity)) {
      auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
      visibilityComp.SetLocalAABB(nodePtr->GetLocalBounds());
      visibilityComp.MarkBoundsDirty();
    }

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

bool SceneGraph::DestroyNode(SceneRegistry &registry, Entity entity)
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

  // 清理VisibilityComponent相关状态
  if (registry.HasComponent<VisibilityComponent>(entity)) {
    auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
    visibilityComp.SetVisible(false);  // 标记为不可见
  }

  // 从映射表中移除
  m_entityToNodeMap.erase(it);

  // 从脏节点列表中移除
  m_dirtyNodes.erase(std::remove(m_dirtyNodes.begin(), m_dirtyNodes.end(), entity),
                     m_dirtyNodes.end());

  m_logger->debug("Destroyed scene node for entity {}", entity.GetUUIDString());
  return true;
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

size_t SceneGraph::QueryVisibleCount(SceneRegistry &registry,
                                     const Frustum &frustum,
                                     uint32_t visibilityMask)
{
  QueryVisibleNodes(registry, frustum, visibilityMask);
  return m_visibleNodeCount;
}

size_t SceneGraph::GetVisibleNodeCount() const
{
  return m_visibleNodeCount;
}

std::vector<SceneNode *> SceneGraph::QueryVisibleNodes(SceneRegistry &registry,
                                                       const Frustum &frustum,
                                                       uint32_t visibilityMask)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_spatialPartition) {
    m_logger->warn("Invalid spatial partition, cannot QueryVisibleNodes.");
    return {};
  }

  std::vector<SceneNode *> results;

  // 第一阶段：使用空间划分结构进行粗粒度剔除
  std::vector<SceneNode *> potentiallyVisibleNodes;
  int spatialResult = m_spatialPartition->FrustumCull(frustum, potentiallyVisibleNodes);

  if (potentiallyVisibleNodes.empty()) {
    m_visibleNodeCount = 0;
    m_logger->trace("None visible node after FrustumCull");
    return {};
  }
  m_logger->trace("{} visible nodes after FrustumCull", potentiallyVisibleNodes.size());

  // 第二阶段：细粒度可见性检查（结合VisibilityComponent）
  m_visibleNodeCount = 0;

  for (SceneNode *node : potentiallyVisibleNodes) {
    Entity entity = node->GetEntity();

    // 检查节点是否有效且具有VisibilityComponent
    if (!entity.IsValid()) {
      continue;
    }
    if (registry.HasComponent<VisibilityComponent>(entity)) {
      auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);

      // 检查可见性掩码匹配
      if (!visibilityComp.MatchesMask(visibilityMask)) {
        continue;  // 掩码不匹配，跳过
      }

      // 检查手动覆盖的可见性状态
      if (!visibilityComp.IsVisible()) {
        continue;  // 手动设置为不可见
      }

      // 执行精确的视锥体裁剪测试
      IntersectionType intersection = visibilityComp.TestFrustum(frustum);

      if (intersection != IntersectionType::Outside) {
        // 节点可见，添加到结果列表
        results.push_back(node);
        m_visibleNodeCount++;

        // 更新VisibilityComponent的可见性状态
        visibilityComp.SetVisible(true);  // 确保状态一致
      }
      else {
        // 节点不可见，更新状态
        visibilityComp.SetVisible(false);
      }
    }
    else {
      // 没有VisibilityComponent的节点，使用保守估计
      // 检查世界包围盒与视锥体的相交测试
      const AABB &worldAABB = node->GetWorldBounds();
      IntersectionType intersection = frustum.TestAABB(worldAABB);

      if (intersection != IntersectionType::Outside) {
        results.push_back(node);
        m_visibleNodeCount++;
      }
    }
  }
  m_logger->debug("QueryVisibleNodes complete: find {} visible nodes（in {} nodes of all）",
                  m_visibleNodeCount,
                  m_entityToNodeMap.size());

  return results;
}

std::vector<SceneNode *> SceneGraph::QueryRaycast(SceneRegistry &registry,
                                                  const Ray &ray,
                                                  uint32_t visibilityMask)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> results;
  if (!m_spatialPartition) {
    m_logger->warn("Invalid spatial partition, cannot Raycast.");
    return results;
  }

  float distance;  // 用于记录Ray行进距离的临时变量

  // 第一阶段：使用空间划分结构进行粗检测
  std::vector<SceneNode *> potentialHits;
  if (m_spatialPartition->Raycast(ray, potentialHits)) {
    // 第二阶段：精确检测和可见性过滤
    for (SceneNode *node : potentialHits) {
      Entity entity = node->GetEntity();

      // 检查节点可见性
      if (!IsNodeVisible(registry, entity, visibilityMask)) {
        continue;
      }
      // 如果有VisibilityComponent，使用其世界包围盒进行精确检测
      if (registry.HasComponent<VisibilityComponent>(entity)) {
        auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
        const AABB &worldAABB = visibilityComp.GetWorldAABB();

        // 精确的射线与AABB相交测试
        if (SpatialPartition::RayIntersectsAABB(ray, worldAABB, distance)) {
          results.push_back(node);
        }
      }
      else {
        // 没有VisibilityComponent，使用SceneNode的世界包围盒
        const AABB &worldAABB = node->GetWorldBounds();
        if (SpatialPartition::RayIntersectsAABB(ray, worldAABB, distance)) {
          results.push_back(node);
        }
      }
    }
  }
  m_logger->trace("QueryRaycast complete: find {} visible nodes", results.size());
  return results;
}

bool SceneGraph::QueryRaycastFirst(SceneRegistry &registry,
                                   const Ray &ray,
                                   SceneNode *&result,
                                   float &distance,
                                   uint32_t visibilityMask)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  result = nullptr;
  distance = FLT_MAX;
  if (!m_spatialPartition) {
    m_logger->warn("Invalid spatial partition, cannot Raycast.");
    return false;
  }
  // 获取所有潜在命中节点
  std::vector<SceneNode *> potentialHits;
  if (!m_spatialPartition->Raycast(ray, potentialHits)) {
    return false;
  }
  SceneNode *closestNode = nullptr;
  float closestDistance = FLT_MAX;
  // 查找最近的可见节点
  for (SceneNode *node : potentialHits) {
    Entity entity = node->GetEntity();

    // 检查节点可见性
    if (!IsNodeVisible(registry, entity, visibilityMask)) {
      continue;
    }

    float hitDistance = FLT_MAX;
    bool hit = false;
    // 精确的射线与包围盒相交测试
    if (registry.HasComponent<VisibilityComponent>(entity)) {
      auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
      hit = SpatialPartition::RayIntersectsAABB(ray, visibilityComp.GetWorldAABB(), hitDistance);
    }
    else {
      hit = SpatialPartition::RayIntersectsAABB(ray, node->GetWorldBounds(), hitDistance);
    }
    if (hit && hitDistance < closestDistance) {
      closestNode = node;
      closestDistance = hitDistance;
    }
  }
  if (closestNode) {
    result = closestNode;
    distance = closestDistance;
    return true;
  }
  return false;
}

std::vector<SceneNode *> SceneGraph::QuerySphere(SceneRegistry &registry,
                                                 const Sphere &sphere,
                                                 uint32_t visibilityMask)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> results;
  if (!m_spatialPartition) {
    m_logger->warn("Invalid spatial partition, cannot QuerySphere.");
    return results;
  }
  // 第一阶段：使用空间划分结构进行粗检测
  std::vector<SceneNode *> potentialHits;
  size_t hitCount = m_spatialPartition->SphereQuery(sphere, potentialHits);

  if (hitCount > 0) {
    // 第二阶段：精确检测和可见性过滤
    for (SceneNode *node : potentialHits) {
      Entity entity = node->GetEntity();

      // 检查节点可见性
      if (!IsNodeVisible(registry, entity, visibilityMask)) {
        continue;
      }
      // 精确的球体与包围盒相交测试
      bool intersects = false;
      if (registry.HasComponent<VisibilityComponent>(entity)) {
        auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
        const AABB &worldAABB = visibilityComp.GetWorldAABB();
        intersects = BoundingVolumes::SphereIntersectsAABB(sphere, worldAABB);
      }
      else {
        const AABB &worldAABB = node->GetWorldBounds();
        intersects = BoundingVolumes::SphereIntersectsAABB(sphere, worldAABB);
      }
      if (intersects) {
        results.push_back(node);
      }
    }
  }
  m_logger->trace("QuerySphere complete: find {} visible nodes", results.size());
  return results;
}

std::vector<SceneNode *> SceneGraph::QueryAABB(SceneRegistry &registry,
                                               const AABB &aabb,
                                               uint32_t visibilityMask)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> results;
  if (!m_spatialPartition) {
    m_logger->warn("Invalid spatial partition, cannot QueryAABB.");
    return results;
  }
  // 第一阶段：使用空间划分结构进行粗检测
  std::vector<SceneNode *> potentialHits;
  size_t hitCount = m_spatialPartition->AABBQuery(aabb, potentialHits);

  if (hitCount > 0) {
    // 第二阶段：精确检测和可见性过滤
    for (SceneNode *node : potentialHits) {
      Entity entity = node->GetEntity();

      // 检查节点可见性
      if (!IsNodeVisible(registry, entity, visibilityMask)) {
        continue;
      }
      // 精确的AABB与AABB相交测试
      bool intersects = false;
      if (registry.HasComponent<VisibilityComponent>(entity)) {
        auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
        const AABB &worldAABB = visibilityComp.GetWorldAABB();
        intersects = BoundingVolumes::AABBIntersectsAABB(aabb, worldAABB);
      }
      else {
        const AABB &worldAABB = node->GetWorldBounds();
        intersects = BoundingVolumes::AABBIntersectsAABB(aabb, worldAABB);
      }
      if (intersects) {
        results.push_back(node);
      }
    }
  }
  m_logger->trace("QueryAABB complete: find {} visible nodes", results.size());
  return results;
}

// ==================== 节点更新接口 ====================
void SceneGraph::UpdateNodeBounds(SceneRegistry &registry, Entity entity, const AABB &localBounds)
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

void SceneGraph::Update(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 更新主相机的视锥体和可见性掩码

   // 更新所有脏节点
  if (m_dirtyNodes.empty()) {
    return;
  }

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

bool SceneGraph::IsNodeVisible(SceneRegistry &registry,
                               Entity entity,
                               uint32_t visibilityMask) const
{
  if (!entity.IsValid()) {
    return false;
  }

  if (registry.HasComponent<VisibilityComponent>(entity)) {
    auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
    return visibilityComp.IsVisible() && visibilityComp.MatchesMask(visibilityMask);
  }

  // 没有VisibilityComponent的节点默认可见
  return true;
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