#include "spatial_partition_manager.h"
#include "scene_core/scene_registry.h"
#include "visibility_component.h"

namespace mite {
SpatialPartitionManager::SpatialPartitionManager(SpatialPartitionType spatialPartitionType)
    : m_spatialPartitionType(spatialPartitionType)
{
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph Spatial Partition Manager");

  // 初始化空间划分结构（默认BVH）
  InitializeSpatialPartition();
}

void SpatialPartitionManager::Clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 清空空间划分结构
  if (m_spatialPartition) {
    m_spatialPartition->Clear();
  }
}

// ==================== 节点增删管理接口 ====================
void SpatialPartitionManager::RemoveNodeFromSpatialPartition(SceneNode *node)
{
  if (m_spatialPartition && node) {
    m_spatialPartition->Remove(node);
  }
}

void SpatialPartitionManager::AddNodeToSpatialPartition(SceneNode *node)
{
  if (m_spatialPartition && node) {
    m_spatialPartition->Insert(node);
  }
}

void SpatialPartitionManager::Update(SceneNode *node)
{
  m_spatialPartition->Update(node);
}

// ==================== 空间划分管理接口 ====================
void SpatialPartitionManager::SetSpatialPartitionType(SpatialPartitionType type)
{
  if (m_spatialPartitionType == type) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  m_spatialPartitionType = type;
  InitializeSpatialPartition();

  m_logger->info("Spatial partition type changed to: {}", GetSpatialPartitionTypeName(type));
}

SpatialPartitionType SpatialPartitionManager::GetSpatialPartitionType() const
{
  return m_spatialPartitionType;
}

void SpatialPartitionManager::RebuildSpatialPartition(std::vector<SceneNode *> nodelist)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    m_logger->warn("Cannot rebuild null spatial partition");
    return;
  }

  m_spatialPartition->Clear();

  // 重新添加所有节点
  for (const auto node : nodelist) {
    AddNodeToSpatialPartition(node);
  }

  m_logger->debug("Rebuilt spatial partition with {} nodes", nodelist.size());
}

std::string SpatialPartitionManager::GetSpatialPartitionStats() const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_spatialPartition) {
    return "Spatial partition not initialized";
  }

  return m_spatialPartition->GetStats();
}

void SpatialPartitionManager::DebugDraw(std::function<void(const AABB &, int depth)> drawCallback)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_spatialPartition && drawCallback) {
    m_spatialPartition->DebugDraw(drawCallback);
  }
}

// ==================== 空间查询接口 ====================
size_t SpatialPartitionManager::GetVisibleNodeCount() const
{
  return m_visibleNodeCount;
}

std::vector<SceneNode *> SpatialPartitionManager::QueryVisibleNodes(SceneRegistry &registry,
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
    //m_logger->trace("None visible node after FrustumCull");
    return {};
  }
  //m_logger->trace("{} visible nodes after FrustumCull", potentiallyVisibleNodes.size());

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
  //m_logger->debug("QueryVisibleNodes complete: find {} visible nodes", m_visibleNodeCount);

  return results;
}

std::vector<SceneNode *> SpatialPartitionManager::QueryRaycast(SceneRegistry &registry,
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
      if (!node->IsNodeVisible(registry, visibilityMask)) {
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

bool SpatialPartitionManager::QueryRaycastFirst(SceneRegistry &registry,
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
    if (!node->IsNodeVisible(registry, visibilityMask)) {
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

std::vector<SceneNode *> SpatialPartitionManager::QuerySphere(SceneRegistry &registry,
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
      if (!node->IsNodeVisible(registry, visibilityMask)) {
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

std::vector<SceneNode *> SpatialPartitionManager::QueryAABB(SceneRegistry &registry,
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
      if (!node->IsNodeVisible(registry, visibilityMask)) {
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

// ==================== 私有工具方法 ====================

void SpatialPartitionManager::InitializeSpatialPartition()
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
}  // namespace mite