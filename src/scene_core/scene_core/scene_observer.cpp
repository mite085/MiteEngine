#include "scene_observer.h"

namespace mite {

SceneObserver::SceneObserver(SceneRegistry &registry) : m_Registry(registry)
{
  // 确保IDComponent总是被跟踪（因为它是实体标识的关键组件）
  m_Registry.RegisterComponentCallback<IDComponent>(
      [this](Entity entity, IDComponent &comp) { OnComponentAdded(entity, comp); },
      [this](Entity entity, IDComponent &comp) { OnComponentRemoved(entity, comp); },
      [this](Entity entity, IDComponent &comp) { OnComponentChanged(entity, comp); });

  SetupCallbacks();
}

SceneObserver::~SceneObserver()
{
  StopTracking();
  CleanupCallbacks();
}

size_t SceneObserver::RegisterCallback(const SceneObserverCallback &callback)
{
  size_t id = m_NextCallbackId++;
  m_Callbacks[id] = callback;
  return id;
}

void SceneObserver::UnregisterCallback(size_t callbackId)
{
  m_Callbacks.erase(callbackId);
}

void SceneObserver::StartTracking()
{
  m_IsTracking = true;
  m_ChangeEvents.clear();
  m_DirtyEntities.clear();
}

void SceneObserver::StopTracking()
{
  m_IsTracking = false;
}

std::vector<SceneChangeEvent> SceneObserver::FlushChanges()
{
  std::vector<SceneChangeEvent> changes;
  std::swap(changes, m_ChangeEvents);
  m_DirtyEntities.clear();
  return changes;
}

bool SceneObserver::IsEntityDirty(Entity entity) const
{
  return m_DirtyEntities.find(entity) != m_DirtyEntities.end();
}

void SceneObserver::SetupCallbacks()
{
  // 设置实体生命周期回调
  m_EntityCreatedCallback = m_Registry.OnEntityCreated(
      [this](Entity entity) { OnEntityCreated(entity); });

  m_EntityDestroyedCallback = m_Registry.OnEntityDestroyed(
      [this](Entity entity) { OnEntityDestroyed(entity); });
}

void SceneObserver::CleanupCallbacks()
{
  // 清理所有注册的回调
  m_Registry.UnregisterEntityCreatedCallback(m_EntityCreatedCallback);
  m_Registry.UnregisterEntityDestroyedCallback(m_EntityDestroyedCallback);

  // 清理组件回调
  for (auto &[id, data] : m_ComponentCallbacks) {
    m_Registry.UnregisterComponentCallbacks(id);
  }
  m_ComponentCallbacks.clear();
}

void SceneObserver::NotifyCallbacks(const SceneChangeEvent &event)
{
  for (auto &[id, callback] : m_Callbacks) {
    if (callback) {
      callback(event);
    }
  }
}

void SceneObserver::OnEntityCreated(Entity entity)
{
  if (!m_IsTracking)
    return;

  SceneChangeEvent event;
  event.changeType = SceneChangeType::ENTITY_CREATED;
  event.entity = entity;

  m_ChangeEvents.push_back(event);
  m_DirtyEntities.insert(entity);

  NotifyCallbacks(event);
}

void SceneObserver::OnEntityDestroyed(Entity entity)
{
  if (!m_IsTracking)
    return;

  SceneChangeEvent event;
  event.changeType = SceneChangeType::ENTITY_DESTROYED;
  event.entity = entity;

  m_ChangeEvents.push_back(event);
  m_DirtyEntities.erase(entity);  // 实体被销毁，从脏集合中移除

  NotifyCallbacks(event);
}

template<typename T> void SceneObserver::OnComponentAdded(Entity entity, T &component)
{
  if (!m_IsTracking)
    return;

  SceneChangeEvent event;
  event.changeType = SceneChangeType::COMPONENT_ADDED;
  event.entity = entity;
  event.componentType = ComponentTypeID<T>();

  // 存储组件快照（如果需要）
  if constexpr (std::is_copy_constructible_v<T>) {
    event.newData = std::make_shared<T>(component);
  }

  m_ChangeEvents.push_back(event);
  m_DirtyEntities.insert(entity);

  NotifyCallbacks(event);
}

template<typename T> void SceneObserver::OnComponentRemoved(Entity entity, T &component)
{
  if (!m_IsTracking)
    return;

  SceneChangeEvent event;
  event.changeType = SceneChangeType::COMPONENT_REMOVED;
  event.entity = entity;
  event.componentType = ComponentTypeID<T>();

  // 存储组件快照（如果需要）
  if constexpr (std::is_copy_constructible_v<T>) {
    event.oldData = std::make_shared<T>(component);
  }

  m_ChangeEvents.push_back(event);
  m_DirtyEntities.insert(entity);

  NotifyCallbacks(event);
}

template<typename T> void SceneObserver::OnComponentChanged(Entity entity, T &component)
{
  if (!m_IsTracking)
    return;

  SceneChangeEvent event;
  event.changeType = SceneChangeType::COMPONENT_CHANGED;
  event.entity = entity;
  event.componentType = ComponentTypeID<T>();

  // 对于可复制的组件，可以存储新旧值
  if constexpr (std::is_copy_constructible_v<T>) {
    // 注意：实际实现中可能需要更高效的方式来存储变更
    event.oldData = std::make_shared<T>(component);  // 这里简化了，实际应该存储旧值
    event.newData = std::make_shared<T>(component);
  }

  m_ChangeEvents.push_back(event);
  m_DirtyEntities.insert(entity);

  NotifyCallbacks(event);
}
};
