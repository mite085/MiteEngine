#include "scene.h"
#include "scene_observer.h"
#include "scene_core_components/component_headers.h"
namespace mite {

SceneObserver::SceneObserver(SceneRegistry &registry) : m_Registry(registry)
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Scene Observer");
  m_Logger->trace("Created scene observer system");
  // 订阅事件
  m_EventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));
  m_EventSubscriptions.Subscribe<EntityParentChangedEvent>(BIND_DISPATCH_FN(MarkEntityModified));
  m_EventSubscriptions.Subscribe<EntityPreDestroyedEvent>(BIND_DISPATCH_FN(OnEntityPreDestroyed));
}

SceneObserver::~SceneObserver()
{
  // 清理时取消回调注册
  m_EventSubscriptions.UnsubscribeAll();
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
  //for (auto &entity : m_CreatedEntities) {
  //  dispatcher.Dispatch<EntityCreatedEvent>(
  //      [this](EntityCreatedEvent &e) { return OnEntityCreated(e.GetEntity()); });

  //  // 若有需要，通知ComponentSystemManager检查新实体的组件
  //  // (目前ComponentSystemManager不维护任何和entity相关的内容)
  //  //m_Scene.lock()->GetComponentSystemManager().OnEntityCreated(entity);
  //}

  //// 处理实体改变事件
  //for (auto &entity : m_ModifiedEntities) {
  //  dispatcher.Dispatch<EntityParentChangedEvent>(
  //      [this](EntityParentChangedEvent &e) { return MarkEntityModified(e.GetEntity()); });
  //}

  //// 处理实体销毁事件
  //for (auto &entity : m_DestroyedEntities) {
  //  dispatcher.Dispatch<EntityDestroyedEvent>(
  //      [this](EntityDestroyedEvent &e) { return OnEntityDestroyed(e.GetEntity()); });
  //}

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

void SceneObserver::MarkEntityModified(EntityParentChangedEvent &e)
{
  if (m_IsObserving) {
    m_ModifiedEntities.push_back(e.GetEntity());
  }
}

void SceneObserver::OnEntityCreated(EntityCreatedEvent &e)
{
  if (m_IsObserving)
    m_CreatedEntities.push_back(e.GetEntity());
}

void SceneObserver::OnEntityPreDestroyed(EntityPreDestroyedEvent &e)
{
  if (!m_IsObserving)
    m_DestroyedEntities.push_back(e.GetEntity());
}

};  // namespace mite
