#include "scene.h"
#include "scene_observer.h"
#include "scene_core_components/component_headers.h"
namespace mite {

SceneObserver::SceneObserver(SceneRegistry &registry) : m_Registry(registry)
{
  // 注册到场景的回调
  m_Registry.RegisterCallbackEntityCreated(
      [this](Entity entity) { OnEntityCreated(entity); });

  m_Registry.RegisterCallbackEntityPreDestroyed(
      [this](Entity entity) { OnEntityDestroyed(entity); });
}

SceneObserver::~SceneObserver()
{
  // 清理时取消回调注册
  m_Registry.UnregisterCallbackEntity();
}

void SceneObserver::BeginObservation()
{
  m_IsObserving = true;
  m_CreatedEntities.clear();
  m_DestroyedEntities.clear();
  m_ModifiedEntities.clear();
}

void SceneObserver::EndObservationAndEmitEvents(EventDispatcher &dispatcher)
{
  if (!m_IsObserving)
    return;

  m_IsObserving = false;

  // 处理实体创建事件
  for (auto &entity : m_CreatedEntities) {
    dispatcher.Dispatch<EntityCreatedEvent>(
        [this](EntityCreatedEvent &e) { return OnEntityCreated(e.GetEntity()); });

    // 若有需要，通知ComponentSystemManager检查新实体的组件
    // (目前ComponentSystemManager不维护任何和entity相关的内容)
    //m_Scene.lock()->GetComponentSystemManager().OnEntityCreated(entity);
  }

  // 处理实体改变事件
  for (auto &entity : m_ModifiedEntities) {
    dispatcher.Dispatch<EntityParentChangedEvent>(
        [this](EntityParentChangedEvent &e) { return MarkEntityModified(e.GetEntity()); });
  }

  // 处理实体销毁事件
  for (auto &entity : m_DestroyedEntities) {
    dispatcher.Dispatch<EntityDestroyedEvent>(
        [this](EntityDestroyedEvent &e) { return OnEntityDestroyed(e.GetEntity()); });
  }

  m_CreatedEntities.clear();
  m_DestroyedEntities.clear();
  m_ModifiedEntities.clear();
}

void SceneObserver::Clear()
{
  m_IsObserving = false;
  m_CreatedEntities.clear();
  m_DestroyedEntities.clear();
  m_ModifiedEntities.clear();
}

bool SceneObserver::MarkEntityModified(Entity entity)
{
  if (!m_IsObserving)
    return false;

  m_ModifiedEntities.push_back(entity);
  return true;
}

bool SceneObserver::OnEntityCreated(Entity entity)
{
  if (!m_IsObserving)
    return false;

  m_CreatedEntities.push_back(entity);
  return true;
}

bool SceneObserver::OnEntityDestroyed(Entity entity)
{
  if (!m_IsObserving)
    return false;

  m_DestroyedEntities.push_back(entity);
  return true;
}
};
