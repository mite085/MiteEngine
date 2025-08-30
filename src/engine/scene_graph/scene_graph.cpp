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
    : m_spatialPartitionManager(spatialPartitionType), m_nodeManager(m_spatialPartitionManager)
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

  m_logger->trace("SceneGraph created with spatial partition type: {}",
                  GetSpatialPartitionTypeName(m_spatialPartitionManager.GetSpatialPartitionType()));
}

void SceneGraph::CleanUp(ComponentSystemManager &manager)
{
  m_logger->info("Destroying SceneGraph");

  manager.UnregisterSystem<SceneGraphSystem>();
  manager.UnregisterSystem<HierarchySceneNodeSystem>();
  manager.UnregisterSystem<TransformSceneNodeSystem>();
  manager.UnregisterSystem<VisibilityComponentSystem>();

  m_nodeManager.Clear();
  m_spatialPartitionManager.Clear();

  m_logger->debug("SceneGraph destroyed");
}

// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneGraph::CreateNode(SceneRegistry &registry, Entity entity)
{
  m_nodeManager.CreateNode(registry, entity);
}

bool SceneGraph::DestroyNode(SceneRegistry &registry, Entity entity)
{
  m_nodeManager.DestroyNode(registry, entity);
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneGraph::GetNode(Entity entity) const
{
  return m_nodeManager.GetNode(entity);
}

bool SceneGraph::HasNode(Entity entity) const
{
  return m_nodeManager.GetNode(entity);
}

std::vector<SceneNode *> SceneGraph::GetRootNodes() const
{
  return m_nodeManager.GetRootNodes();
}

std::vector<SceneNode *> SceneGraph::GetAllNodes() const
{
  return m_nodeManager.GetAllNodes();
}

size_t SceneGraph::GetNodeCount() const
{
  return m_nodeManager.GetNodeCount();
}

bool SceneGraph::IsEmpty() const
{
  return m_nodeManager.IsEmpty();
}

// ==================== 场景树操作接口（编辑器支持） ====================
bool SceneGraph::SetParent(SceneNode *node, SceneNode *newParent)
{
  return m_nodeManager.SetParent(node, newParent);
}

std::string SceneGraph::GetNodePath(SceneNode *node) const
{
  return m_nodeManager.GetNodePath(node);
}

SceneNode *SceneGraph::FindNodeByPath(const std::string &path) const
{
  return m_nodeManager.FindNodeByPath(path);
}

void SceneGraph::TraverseTree(std::function<bool(SceneNode *)> callback) const
{
  m_nodeManager.TraverseTree(callback);
}

// ==================== 空间划分管理接口 ====================
void SceneGraph::SetSpatialPartitionType(SpatialPartitionType type)
{
  m_spatialPartitionManager.SetSpatialPartitionType(type);
}

SpatialPartitionType SceneGraph::GetSpatialPartitionType() const
{
  return m_spatialPartitionManager.GetSpatialPartitionType();
}

void SceneGraph::RebuildSpatialPartition(std::vector<SceneNode *> nodelist)
{
  m_spatialPartitionManager.RebuildSpatialPartition(nodelist);
}

std::string SceneGraph::GetSpatialPartitionStats() const
{
  return m_spatialPartitionManager.GetSpatialPartitionStats();
}

void SceneGraph::DebugDraw(std::function<void(const AABB &, int depth)> drawCallback)
{
  m_spatialPartitionManager.DebugDraw(drawCallback);
}

// ==================== 空间查询接口 ====================

size_t SceneGraph::QueryVisibleCount(SceneRegistry &registry,
                                     const Frustum &frustum,
                                     uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QueryVisibleNodes(registry, frustum, visibilityMask).size();
}

size_t SceneGraph::GetVisibleNodeCount() const
{
  return m_spatialPartitionManager.GetVisibleNodeCount();
}

std::vector<SceneNode *> SceneGraph::QueryVisibleNodes(SceneRegistry &registry,
                                                       const Frustum &frustum,
                                                       uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QueryVisibleNodes(registry, frustum, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QueryRaycast(SceneRegistry &registry,
                                                  const Ray &ray,
                                                  uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QueryRaycast(registry, ray, visibilityMask);
}

bool SceneGraph::QueryRaycastFirst(SceneRegistry &registry,
                                   const Ray &ray,
                                   SceneNode *&result,
                                   float &distance,
                                   uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QueryRaycastFirst(
      registry, ray, result, distance, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QuerySphere(SceneRegistry &registry,
                                                 const Sphere &sphere,
                                                 uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QuerySphere(registry, sphere, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QueryAABB(SceneRegistry &registry,
                                               const AABB &aabb,
                                               uint32_t visibilityMask)
{
  return m_spatialPartitionManager.QueryAABB(registry, aabb, visibilityMask);
}

// ==================== 节点更新接口 ====================
void SceneGraph::UpdateNodeBounds(SceneRegistry &registry, Entity entity, const AABB &localBounds)
{
  m_nodeManager.UpdateNodeBounds(registry, entity, localBounds);
}

void SceneGraph::MarkNodeDirty(Entity entity)
{
  m_nodeManager.MarkNodeDirty(entity);
}

void SceneGraph::Update(SceneRegistry &registry)
{
  m_nodeManager.Update(registry);
}

// ==================== 序列化支持 ====================
bool SceneGraph::Serialize(std::ostream &output) const
{
  // TODO: 实现完整的场景图序列化
  // 目前先预留接口
  m_logger->info("SceneGraph serialization called (not implemented)");
  return !output.fail();
}

bool SceneGraph::Deserialize(std::istream &input)
{
  // TODO: 实现完整的场景图反序列化
  // 目前先预留接口
  m_logger->info("SceneGraph deserialization called (not implemented)");
  return !input.fail();
}
}  // namespace mite