#include "transform_system.h"
#include "bounding_volumes.h"
#include "scene_core/component_system_manager.h"
#include "scene_core/scene_registry.h"

namespace mite {

TransformSystem::TransformSystem()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite TransformSystem");
  m_Logger->trace("TransformSystem created");
}

Component::Family TransformSystem::GetExecutionOrder() const
{
  // TODO: 应当在TransformComponent处理完脏标记后执行
  return Component::Family::Core;
}

void TransformSystem::Initialize()
{
  m_Logger->info("Initializing TransformSystem");

  // TransformSystem并非继承自DirtyComponentSystem，组件添加/删除事件需要单独订阅
  m_EventSubscriptions.Subscribe<ComponentAddedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentAdded));
  m_EventSubscriptions.Subscribe<ComponentRemovedEvent<TransformComponent>>(
      BIND_DISPATCH_FN(OnTransformComponentRemoved));
  m_EventSubscriptions.Subscribe<TransformUpdatedEvent>(BIND_DISPATCH_FN(OnTransformUpdated));
  m_EventSubscriptions.Subscribe<ParentChangedEvent>(BIND_DISPATCH_FN(OnParentChanged));

  m_Logger->debug("TransformSystem initialized - event subscriptions complete");
}

void TransformSystem::Update(float deltaTime, SceneRegistry &registry)
{
  ProcessPendingSync(registry);
}

void TransformSystem::Shutdown()
{
  m_Logger->info("Shutting down TransformSystem");
  m_EventSubscriptions.UnsubscribeAll();

  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap.clear();
  m_pendingSyncEntities.clear();
}

std::vector<std::type_index> TransformSystem::GetComponentTypes() const
{
  return {typeid(TransformComponent)};
}

std::vector<std::type_index> TransformSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem), typeid(HierarchyComponentSystem)};
}

void TransformSystem::RegisterSceneNode(Entity entity, SceneNode *node)
{
  if (!node)
    return;

  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap[entity] = node;
  m_pendingSyncEntities.push_back(entity);
}

void TransformSystem::UnregisterSceneNode(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_entityToNodeMap.erase(entity);

  m_pendingSyncEntities.erase(
      std::remove(m_pendingSyncEntities.begin(), m_pendingSyncEntities.end(), entity),
      m_pendingSyncEntities.end());
}

SceneNode *TransformSystem::GetSceneNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_entityToNodeMap.find(entity);
  return it != m_entityToNodeMap.end() ? it->second : nullptr;
}

void TransformSystem::SyncAllComponentsToNodes(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (registry.HasComponent<TransformComponent>(entity)) {
      SyncComponentToNode(registry, entity, node);
    }
  }
}

void TransformSystem::MarkEntityForSync(Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_pendingSyncEntities.push_back(entity);
}

bool TransformSystem::OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

bool TransformSystem::OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e)
{
  Entity entity = e.GetEntity();

  std::lock_guard<std::mutex> lock(m_mutex);
  m_pendingSyncEntities.erase(
      std::remove(m_pendingSyncEntities.begin(), m_pendingSyncEntities.end(), entity),
      m_pendingSyncEntities.end());

  e.Handled();
  return true;
}

bool TransformSystem::OnTransformUpdated(TransformUpdatedEvent &e)
{
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

bool TransformSystem::OnParentChanged(ParentChangedEvent &e)
{
  // 简化实现：只标记当前实体，子节点会在变换传播时自动标记
  MarkEntityForSync(e.GetEntity());
  e.Handled();
  return true;
}

void TransformSystem::ProcessPendingSync(SceneRegistry &registry)
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

void TransformSystem::SyncComponentToNode(SceneRegistry &registry, Entity entity, SceneNode *node)
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
