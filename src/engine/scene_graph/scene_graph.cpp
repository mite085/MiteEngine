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
    : m_SpatialPartitionManager(spatialPartitionType), m_NodeManager(m_SpatialPartitionManager)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph");
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

  m_Logger->trace("SceneGraph created with spatial partition type: {}",
                  GetSpatialPartitionTypeName(m_SpatialPartitionManager.GetSpatialPartitionType()));
}

void SceneGraph::CleanUp(ComponentSystemManager &manager)
{
  m_Logger->info("Destroying SceneGraph");

  manager.UnregisterSystem<SceneGraphSystem>();
  manager.UnregisterSystem<HierarchySceneNodeSystem>();
  manager.UnregisterSystem<TransformSceneNodeSystem>();
  manager.UnregisterSystem<VisibilityComponentSystem>();

  m_NodeManager.Clear();
  m_SpatialPartitionManager.Clear();

  m_Logger->debug("SceneGraph destroyed");
}

// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneGraph::CreateNode(SceneRegistry &registry, Entity entity)
{
  return m_NodeManager.CreateNode(registry, entity);
}

bool SceneGraph::DestroyNode(SceneRegistry &registry, Entity entity)
{
  return m_NodeManager.DestroyNode(registry, entity);
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneGraph::GetNode(Entity entity) const
{
  return m_NodeManager.GetNode(entity);
}

bool SceneGraph::HasNode(Entity entity) const
{
  return m_NodeManager.GetNode(entity);
}

std::vector<SceneNode *> SceneGraph::GetRootNodes() const
{
  return m_NodeManager.GetRootNodes();
}

std::vector<SceneNode *> SceneGraph::GetAllNodes() const
{
  return m_NodeManager.GetAllNodes();
}

size_t SceneGraph::GetNodeCount() const
{
  return m_NodeManager.GetNodeCount();
}

bool SceneGraph::IsEmpty() const
{
  return m_NodeManager.IsEmpty();
}

// ==================== 场景树操作接口（编辑器支持） ====================
bool SceneGraph::SetParent(SceneNode *node, SceneNode *newParent)
{
  return m_NodeManager.SetParent(node, newParent);
}

std::string SceneGraph::GetNodePath(SceneNode *node) const
{
  return m_NodeManager.GetNodePath(node);
}

SceneNode *SceneGraph::FindNodeByPath(const std::string &path) const
{
  return m_NodeManager.FindNodeByPath(path);
}

void SceneGraph::TraverseTree(std::function<bool(SceneNode *)> callback) const
{
  m_NodeManager.TraverseTree(callback);
}

// ==================== 空间划分管理接口 ====================
void SceneGraph::SetSpatialPartitionType(SpatialPartitionType type)
{
  m_SpatialPartitionManager.SetSpatialPartitionType(type);
}

SpatialPartitionType SceneGraph::GetSpatialPartitionType() const
{
  return m_SpatialPartitionManager.GetSpatialPartitionType();
}

void SceneGraph::RebuildSpatialPartition(std::vector<SceneNode *> nodelist)
{
  m_SpatialPartitionManager.RebuildSpatialPartition(nodelist);
}

std::string SceneGraph::GetSpatialPartitionStats() const
{
  return m_SpatialPartitionManager.GetSpatialPartitionStats();
}

void SceneGraph::DebugDraw(std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback)
{
  m_SpatialPartitionManager.DebugDraw(drawCallback);
}

// ==================== 空间查询接口 ====================

size_t SceneGraph::QueryVisibleCount(SceneRegistry &registry,
                                     const Frustum &frustum,
                                     uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QueryVisibleNodes(registry, frustum, visibilityMask).size();
}

size_t SceneGraph::GetVisibleNodeCount() const
{
  return m_SpatialPartitionManager.GetVisibleNodeCount();
}

std::vector<SceneNode *> SceneGraph::QueryVisibleNodes(SceneRegistry &registry,
                                                       const Frustum &frustum,
                                                       uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QueryVisibleNodes(registry, frustum, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QueryRaycast(SceneRegistry &registry,
                                                  const Ray &ray,
                                                  uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QueryRaycast(registry, ray, visibilityMask);
}

bool SceneGraph::QueryRaycastFirst(SceneRegistry &registry,
                                   const Ray &ray,
                                   SceneNode *&result,
                                   float &distance,
                                   uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QueryRaycastFirst(
      registry, ray, result, distance, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QuerySphere(SceneRegistry &registry,
                                                 const BoundingVolumeSphere &sphere,
                                                 uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QuerySphere(registry, sphere, visibilityMask);
}

std::vector<SceneNode *> SceneGraph::QueryAABB(SceneRegistry &registry,
                                               const BoundingVolumeAABB &aabb,
                                               uint32_t visibilityMask)
{
  return m_SpatialPartitionManager.QueryAABB(registry, aabb, visibilityMask);
}

// ==================== 节点更新接口 ====================
void SceneGraph::UpdateNodeBounds(SceneRegistry &registry, Entity entity, const BoundingVolumeAABB &localBounds)
{
  m_NodeManager.UpdateNodeBounds(registry, entity, localBounds);
}

void SceneGraph::MarkNodeDirty(Entity entity)
{
  m_NodeManager.MarkNodeDirty(entity);
}

void SceneGraph::Update(SceneRegistry &registry)
{
  m_NodeManager.Update(registry);
}

// ==================== 序列化支持 ====================
bool SceneGraph::Serialize(std::ostream &output) const
{
  // TODO: 实现完整的场景图序列化
  // 目前先预留接口
  m_Logger->info("SceneGraph serialization called (not implemented)");
  return !output.fail();
}

bool SceneGraph::Deserialize(std::istream &input)
{
  // TODO: 实现完整的场景图反序列化
  // 目前先预留接口
  m_Logger->info("SceneGraph deserialization called (not implemented)");
  return !input.fail();
}
}  // namespace mite