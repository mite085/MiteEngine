#include "transform_scene_node_system.h"
#include "bounding_volumes.h"
#include "scene_core/component_system_manager.h"
#include "scene_core/scene_registry.h"

namespace mite {

TransformSceneNodeSystem::TransformSceneNodeSystem()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite TransformSceneNodeSystem");
  m_Logger->trace("TransformSceneNodeSystem created");
}

Component::Family TransformSceneNodeSystem::GetExecutionOrder() const
{
  return Component::Family::Transform;
}

void TransformSceneNodeSystem::Initialize()
{
  m_Logger->info("Initializing TransformSceneNodeSystem");

  // TransformSystem并非继承自DirtyComponentSystem，组件添加/删除事件需要单独订阅
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentAdded));
  m_EventSubscriptions.Subscribe<ComponentRemovedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentRemoved));
  m_EventSubscriptions.Subscribe<TransformUpdatedEvent>(BIND_DISPATCH_FN(OnTransformUpdated));

  m_Logger->debug("TransformSceneNodeSystem initialized - event subscriptions complete");
}

void TransformSceneNodeSystem::Update(float deltaTime, SceneRegistry &registry)
{
  ProcessPendingSync(registry);
}

void TransformSceneNodeSystem::Shutdown()
{
  m_Logger->info("Shutting down TransformSceneNodeSystem");
  m_EventSubscriptions.UnsubscribeAll();

  std::lock_guard<std::mutex> lock(m_Mutex);
  m_PendingSyncEntities.clear();
}

std::vector<std::type_index> TransformSceneNodeSystem::GetComponentTypes() const
{
  return {typeid(TransformComponent)};
}

std::vector<std::type_index> TransformSceneNodeSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem)};// 依赖ECS变换和层级
}

void TransformSceneNodeSystem::SetSceneGraph(SceneGraph *sceneGraph)
{
  m_SceneGraph = sceneGraph;
}

void TransformSceneNodeSystem::SyncAllComponentsToNodes(SceneRegistry &registry)
{
  if (!m_SceneGraph) {
    return;
  }
  // 遍历所有有变换组件的实体
  auto view = registry.GetEntitiesWith<TransformComponent>();
  for (Entity entity : view) {
    if (m_SceneGraph->HasNode(entity)) {
      SyncComponentToNode(registry, entity);
    }
  }
}

void TransformSceneNodeSystem::MarkEntityForSync(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  // 避免重复添加
  if (std::find(m_PendingSyncEntities.begin(), m_PendingSyncEntities.end(), entity) ==
      m_PendingSyncEntities.end())
  {
    m_PendingSyncEntities.push_back(entity);
  }
}

bool TransformSceneNodeSystem::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

bool TransformSceneNodeSystem::OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e)
{
  // 移除组件时不需要特殊处理，SceneGraph会处理节点销毁
  // 此处仅需要维护好PendingSyncEntities即可

  Entity entity = e.GetEntity();

  std::lock_guard<std::mutex> lock(m_Mutex);
  m_PendingSyncEntities.erase(
      std::remove(m_PendingSyncEntities.begin(), m_PendingSyncEntities.end(), entity),
      m_PendingSyncEntities.end());

  e.Handled();
  return true;
}

bool TransformSceneNodeSystem::OnTransformUpdated(TransformUpdatedEvent &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

void TransformSceneNodeSystem::ProcessPendingSync(SceneRegistry &registry)
{
  std::vector<Entity> processingEntities;

  // 处理阶段先交换缓存，后续处理阶段不影响其他事件触发导致的m_pendingSyncEntities修改
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_PendingSyncEntities.empty())
      return;
    processingEntities.swap(m_PendingSyncEntities);
  }

  for (Entity entity : processingEntities) {
    if (registry.IsValid(entity) && registry.HasComponent<TransformComponent>(entity)) {
      SyncComponentToNode(registry, entity);
    }
  }
}

void TransformSceneNodeSystem::SyncComponentToNode(SceneRegistry &registry, Entity entity)
{
  if (!m_SceneGraph || !m_SceneGraph->HasNode(entity)) {
    return;
  }
  try {
    auto &transformComp = registry.GetComponent<TransformComponent>(entity);
    SceneNode *node = m_SceneGraph->GetNode(entity);

    if (node) {
      node->SetLocalTransform(transformComp.GetLocalMatrix());
      node->MarkTransformDirty();
      node->MarkBoundsDirty();

      //m_Logger->debug("Synced transform for entity {}", entity.GetUUIDString());
    }
  }
  catch (const std::exception &e) {
    m_Logger->error("Sync failed for entity {}: {}", entity.GetUUIDString(), e.what());
  }
}

}  // namespace mite
