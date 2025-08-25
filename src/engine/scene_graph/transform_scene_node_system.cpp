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
  // TODO: 应当在TransformComponent处理完脏标记后执行
  return Component::Family::Core;
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
  m_EventSubscriptions.Subscribe<ParentChangedEvent>(BIND_DISPATCH_FN(OnParentChanged));

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

  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap.clear();
  m_pendingSyncEntities.clear();
}

std::vector<std::type_index> TransformSceneNodeSystem::GetComponentTypes() const
{
  return {typeid(TransformComponent)};
}

std::vector<std::type_index> TransformSceneNodeSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem), typeid(HierarchyComponentSystem)};
}

void TransformSceneNodeSystem::RegisterSceneNode(Entity entity, SceneNode *node)
{
  if (!node)
    return;

  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap[entity] = node;
  m_pendingSyncEntities.push_back(entity);
}

void TransformSceneNodeSystem::UnregisterSceneNode(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap.erase(entity);

  m_pendingSyncEntities.erase(
      std::remove(m_pendingSyncEntities.begin(), m_pendingSyncEntities.end(), entity),
      m_pendingSyncEntities.end());
}

SceneNode *TransformSceneNodeSystem::GetSceneNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_entityToNodeMap.find(entity);
  return it != m_entityToNodeMap.end() ? it->second : nullptr;
}

void TransformSceneNodeSystem::SyncAllComponentsToNodes(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (registry.HasComponent<TransformComponent>(entity)) {
      SyncComponentToNode(registry, entity, node);
    }
  }
}

void TransformSceneNodeSystem::MarkEntityForSync(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_pendingSyncEntities.push_back(entity);
}

bool TransformSceneNodeSystem::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

bool TransformSceneNodeSystem::OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e)
{
  Entity entity = e.GetEntity();

  std::lock_guard<std::mutex> lock(m_mutex);
  m_pendingSyncEntities.erase(
      std::remove(m_pendingSyncEntities.begin(), m_pendingSyncEntities.end(), entity),
      m_pendingSyncEntities.end());

  e.Handled();
  return true;
}

bool TransformSceneNodeSystem::OnTransformUpdated(TransformUpdatedEvent &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

bool TransformSceneNodeSystem::OnParentChanged(ParentChangedEvent &e)
{
  // 简化实现：只标记当前实体，子节点会在变换传播时自动标记
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

void TransformSceneNodeSystem::ProcessPendingSync(SceneRegistry &registry)
{
  std::vector<Entity> processingEntities;

  // 处理阶段先交换缓存，后续处理阶段不影响其他事件触发导致的m_pendingSyncEntities修改
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pendingSyncEntities.empty())
      return;
    processingEntities.swap(m_pendingSyncEntities);
  }

  for (Entity entity : processingEntities) {
    if (registry.IsValid(entity)) {
      SceneNode *node = GetSceneNode(entity);
      if (node && registry.HasComponent<TransformComponent>(entity)) {
        SyncComponentToNode(registry, entity, node);
      }
    }
  }
}

void TransformSceneNodeSystem::SyncComponentToNode(SceneRegistry &registry, Entity entity, SceneNode *node)
{
  try {
    auto &transformComp = registry.GetComponent<TransformComponent>(entity);
    node->SetLocalTransform(transformComp.GetLocalMatrix());
    node->MarkTransformDirty();
    node->MarkBoundsDirty();
  }
  catch (const std::exception &e) {
    m_Logger->error("Sync failed for entity {}: {}", entity.GetUUIDString(), e.what());
  }
}

}  // namespace mite
