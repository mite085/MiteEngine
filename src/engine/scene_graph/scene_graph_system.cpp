#include "scene_graph_system.h"

namespace mite {
// ==================== 构造函数 ====================
SceneGraphSystem::SceneGraphSystem() : m_sceneGraph(nullptr)
{
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraphSystem");
  m_logger->trace("SceneGraphSystem created");
}

// ==================== ComponentSystem 接口实现 ====================
Component::Family SceneGraphSystem::GetExecutionOrder() const
{
  // TODO: 在TransformSystem之后执行，确保变换数据已同步
  return Component::Family::Core;
}

void SceneGraphSystem::Initialize()
{
  m_logger->info("Initializing SceneGraphSystem");

  // 订阅ECS事件
  m_eventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));

  m_eventSubscriptions.Subscribe<EntityDestroyedEvent>(BIND_DISPATCH_FN(OnEntityDestroyed));

  m_eventSubscriptions.Subscribe<ComponentAddedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentAdded));

  m_eventSubscriptions.Subscribe<ComponentRemovedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentRemoved));

  m_eventSubscriptions.Subscribe<TransformUpdatedEvent>(BIND_DISPATCH_FN(OnTransformUpdated));

  m_eventSubscriptions.Subscribe<ComponentAddedEvent<MeshComponent>>(
      BIND_DISPATCH_FN(OnMeshComponentAdded));

  m_eventSubscriptions.Subscribe<ComponentRemovedEvent<MeshComponent>>(
      BIND_DISPATCH_FN(OnMeshComponentRemoved));

  m_eventSubscriptions.Subscribe<ParentChangedEvent>(BIND_DISPATCH_FN(OnParentChanged));

  // 清空暂存队列（确保初始化后状态干净）
  m_pendingCreateNodes.clear();
  m_pendingDestroyNodes.clear();
  m_pendingSyncTransforms.clear();
  m_pendingSyncBounds.clear();
  m_pendingParentChanges.clear();

  m_logger->debug("SceneGraphSystem initialized.");
}

void SceneGraphSystem::Update(float deltaTime, SceneRegistry &registry)
{
  if (!m_sceneGraph) {
    return;
  }

  // 处理所有暂存的操作请求
  ProcessPendingOperations(registry);

  // 更新SceneGraph中的脏节点
  m_sceneGraph->UpdateDirtyNodes(registry);

  // 定期输出统计信息（调试用）
  static float statsTimer = 0.0f;
  statsTimer += deltaTime;
  if (statsTimer > 5.0f) {
    m_logger->debug("SceneGraphSystem stats: {}", GetStats());
    statsTimer = 0.0f;
  }
}

void SceneGraphSystem::Shutdown()
{
  m_logger->info("Shutting down SceneGraphSystem");

  // 取消所有事件订阅
  m_eventSubscriptions.UnsubscribeAll();

  // 清空统计信息
  m_stats = {};

  m_logger->debug("SceneGraphSystem shutdown complete");
}

std::vector<std::type_index> SceneGraphSystem::GetComponentTypes() const
{
  return {typeid(TransformComponent), typeid(MeshComponent), typeid(HierarchyComponent)};
}

std::vector<std::type_index> SceneGraphSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem),
          typeid(TransformSceneNodeSystem),
          typeid(MeshComponentSystem),
          typeid(HierarchyComponentSystem)};
}

// ==================== SceneGraph 访问接口 ====================
SceneGraph *SceneGraphSystem::GetSceneGraph() const
{
  return m_sceneGraph;
}

void SceneGraphSystem::SetSceneGraph(SceneGraph *sceneGraph)
{
  m_sceneGraph = sceneGraph;
  if (m_sceneGraph) {
    m_logger->info("SceneGraph service attached");
  }
  else {
    m_logger->warn("SceneGraph service detached");
  }
}

// ==================== 调试和统计接口 ====================
std::string SceneGraphSystem::GetStats() const
{
  std::stringstream ss;
  ss << "NodesCreated=" << m_stats.nodesCreated << ", NodesDestroyed=" << m_stats.nodesDestroyed
     << ", TransformSyncs=" << m_stats.transformSyncs << ", BoundsSyncs=" << m_stats.boundsSyncs;
  return ss.str();
}

// ==================== ECS事件处理回调 ====================
bool SceneGraphSystem::OnEntityCreated(EntityCreatedEvent &e)
{
  Entity entity = e.GetEntity();
  m_pendingCreateNodes.push_back(entity);  // 暂存而不是立即处理
  e.Handled();
  return true;
}

bool SceneGraphSystem::OnEntityDestroyed(EntityDestroyedEvent &e)
{
  Entity entity = e.GetEntity();
  m_pendingDestroyNodes.push_back(entity);  // 暂存而不是立即处理
  e.Handled();
  return true;
}

bool SceneGraphSystem::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e)
{
  Entity entity = e.GetEntity();

  // 暂存创建请求，不立即处理
  m_pendingCreateNodes.push_back(entity);

  e.Handled();
  return true;
}

bool SceneGraphSystem::OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e)
{
  Entity entity = e.GetEntity();

  // 暂存销毁请求
  m_pendingDestroyNodes.push_back(entity);

  e.Handled();
  return true;
}

bool SceneGraphSystem::OnTransformUpdated(TransformUpdatedEvent &e)
{
  Entity entity = e.GetEntity();
  m_pendingSyncTransforms.push_back(entity);  // 暂存同步请求
  e.Handled();
  return true;
}

bool SceneGraphSystem::OnMeshComponentAdded(ComponentAddedEvent<MeshComponent> &e)
{
  Entity entity = e.GetEntity();
  m_pendingSyncBounds.push_back(entity);  // 暂存包围盒同步
  e.Handled();
  return true;
}

bool SceneGraphSystem::OnMeshComponentRemoved(ComponentRemovedEvent<MeshComponent> &e)
{
  Entity entity = e.GetEntity();

  // 暂存销毁检查请求
  m_pendingDestroyNodes.push_back(entity);

  e.Handled();
  return true;
}

bool SceneGraphSystem::OnParentChanged(ParentChangedEvent &e)
{
  m_pendingParentChanges.emplace_back(e.GetEntity(), e.GetNewParent());
  e.Handled();
  return true;
}

// ==================== 内部工具方法 ====================
void SceneGraphSystem::CreateNodeForEntity(SceneRegistry &registry, Entity entity)
{
  if (!m_sceneGraph) {
    m_logger->warn("Cannot create node - SceneGraph service not available");
    return;
  }

  if (m_sceneGraph->HasNode(entity)) {
    m_logger->debug("Scene node already exists for entity {}", entity.GetUUIDString());
    return;
  }

  SceneNode *node = m_sceneGraph->CreateNode(registry, entity);
  if (node) {
    m_stats.nodesCreated++;

    // 立即同步初始数据
    SyncTransformToSceneGraph(registry, entity);
    SyncBoundsToSceneGraph(registry, entity);

    m_logger->debug("Created and synced scene node for entity {}", entity.GetUUIDString());
  }
}

bool SceneGraphSystem::ShouldCreateNodeForEntity(SceneRegistry &registry, Entity entity) const
{
  // 只有拥有变换组件的实体才需要场景节点
  // （因为场景节点主要用于空间变换和渲染）
  return registry.HasComponent<TransformComponent>(entity);
}

void SceneGraphSystem::SyncTransformToSceneGraph(SceneRegistry &registry, Entity entity)
{
  if (!m_sceneGraph || !registry.HasComponent<TransformComponent>(entity)) {
    return;
  }

  try {
    auto &transformComp = registry.GetComponent<TransformComponent>(entity);
    glm::mat4 localTransform = transformComp.GetLocalMatrix();

    m_sceneGraph->UpdateNodeTransform(registry, entity, localTransform);
    m_stats.transformSyncs++;
  }
  catch (const std::exception &e) {
    m_logger->error(
        "Failed to sync transform for entity {}: {}", entity.GetUUIDString(), e.what());
  }
}

void SceneGraphSystem::SyncBoundsToSceneGraph(SceneRegistry &registry, Entity entity)
{
  if (!m_sceneGraph) {
    return;
  }

  AABB localBounds;
  bool hasBounds = false;

  // 尝试从Mesh组件获取包围盒
  if (registry.HasComponent<MeshComponent>(entity)) {
    try {
      auto &meshComp = registry.GetComponent<MeshComponent>(entity);
      if (meshComp.HasMesh()) {
        auto bbox = meshComp.GetBoundingBox();
        localBounds = AABB(bbox.first, bbox.second);
        hasBounds = true;
      }
    }
    catch (const std::exception &e) {
      m_logger->warn(
          "Failed to get mesh bounds for entity {}: {}", entity.GetUUIDString(), e.what());
    }
  }

  // 如果没有Mesh组件，使用默认包围盒
  if (!hasBounds) {
    localBounds = AABB(glm::vec3(-0.5f), glm::vec3(0.5f));
  }

  m_sceneGraph->UpdateNodeBounds(registry, entity, localBounds);
  m_stats.boundsSyncs++;
}

void SceneGraphSystem::HandleParentChange(SceneRegistry &registry, Entity entity, Entity newParent)
{
  if (!m_sceneGraph) {
    return;
  }

  SceneNode *node = m_sceneGraph->GetNode(entity);
  SceneNode *parentNode = nullptr;

  if (newParent.IsValid()) {
    parentNode = m_sceneGraph->GetNode(newParent);
    if (!parentNode) {
      // 如果父节点还没有场景节点，先创建
      CreateNodeForEntity(registry, newParent);
      parentNode = m_sceneGraph->GetNode(newParent);
    }
  }

  if (node) {
    m_sceneGraph->SetParent(node, parentNode);
    m_logger->debug("Updated parent for entity {}", entity.GetUUIDString());
  }
}

void SceneGraphSystem::ProcessPendingOperations(SceneRegistry &registry)
{
  // 1. 先处理节点销毁（避免操作已销毁的节点）
  for (Entity entity : m_pendingDestroyNodes) {
    if (m_sceneGraph && m_sceneGraph->HasNode(entity)) {
      // 检查是否真的需要销毁（没有变换组件）
      if (!ShouldCreateNodeForEntity(registry, entity)) {
        m_sceneGraph->DestroyNode(registry,entity);
        m_stats.nodesDestroyed++;
      }
    }
  }
  m_pendingDestroyNodes.clear();

  // 2. 处理节点创建
  for (Entity entity : m_pendingCreateNodes) {
    if (ShouldCreateNodeForEntity(registry, entity) && m_sceneGraph &&
        !m_sceneGraph->HasNode(entity))
    {
      CreateNodeForEntity(registry, entity);
    }
  }
  m_pendingCreateNodes.clear();

  // 3. 处理其他同步操作（原有的变换、包围盒、父子关系）
  for (Entity entity : m_pendingSyncTransforms) {
    if (m_sceneGraph && m_sceneGraph->HasNode(entity)) {
      SyncTransformToSceneGraph(registry, entity);
    }
  }
  m_pendingSyncTransforms.clear();

  for (Entity entity : m_pendingSyncBounds) {
    if (m_sceneGraph && m_sceneGraph->HasNode(entity)) {
      SyncBoundsToSceneGraph(registry, entity);
    }
  }
  m_pendingSyncBounds.clear();

  for (auto &[entity, newParent] : m_pendingParentChanges) {
    if (m_sceneGraph && m_sceneGraph->HasNode(entity)) {
      HandleParentChange(registry, entity, newParent);
    }
  }
  m_pendingParentChanges.clear();
}
}  // namespace mite