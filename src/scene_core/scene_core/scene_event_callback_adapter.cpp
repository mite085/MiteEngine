#include "scene_event_callback_adapter.h"
#include "scene_core_components/component_headers.h"
namespace mite {
SceneEventCallbackAdapter::SceneEventCallbackAdapter(SceneRegistry *registry) : CallbackAdapter()
{
  RegisterCallbacks(registry);
}
SceneEventCallbackAdapter::~SceneEventCallbackAdapter()
{
  UnregisterCallbacks();
}

void SceneEventCallbackAdapter::RegisterCallbacks(SceneRegistry *registry)
{
  m_Registry = registry;

  // 此处仅注册实体回调
  RegisterEntityCallbacks();

  // 组件回调由ComponentSystemManager的
  // RegisterSystem方法负责注册：
  // m_Adapter.RegisterComponentCallbacks<U>();
  //
  // 好处是：
  // 无论后续添加多少新的Component类型,
  // 均只受Manager管理，无需在此处添加新的注册行为。
  //
  // 而坏处是：
  // 多了一层ComponentSystemManager对Adapter的依赖，
  // 需要在初始化时注意两者创建顺序，
  // 且Adapter的构造与m_Registry的获取绑定。
}

void SceneEventCallbackAdapter::UnregisterCallbacks()
{
  // 此处仅注销实体回调，
  
  UnregisterCallbackEntity();

  // 组件回调由ComponentSystemManager的
  // RegisterSystem方法负责注销：
  // m_Adapter.UnregisterComponentCallbacks<U>();
  // 
  // 但维护的列表需要手动删除
  m_ConstructCallbacks.clear();
  m_UpdateCallbacks.clear();
  m_DestroyCallbacks.clear();
}

void SceneEventCallbackAdapter::RegisterEntityCallbacks()
{
  // 实体创建事件
  RegisterCallbackEntityCreated([this](Entity entity) {
    EntityCreatedEvent event(entity);
    EventBus::Get().Post(event);
  });

  // 实体销毁事件
  RegisterCallbackEntityDestroyed([this](Entity entity) {
    EntityDestroyedEvent event(entity);
    EventBus::Get().Post(event);
  });
}

// 3. 实体事件回调相关 ===============================================

void SceneEventCallbackAdapter::RegisterCallbackEntityCreated(EntityCallback callback)
{
  // 连接到EnTT的回调系统
  m_Registry->GetUnderlyingRegistry()
      .on_construct<entt::entity>()
      .connect<&SceneEventCallbackAdapter::InvokeEntityCreated>(
      this);
}

void SceneEventCallbackAdapter::RegisterCallbackEntityDestroyed(EntityCallback callback)
{
  // 连接到EnTT的回调系统
  m_Registry->GetUnderlyingRegistry()
      .on_destroy<entt::entity>()
      .connect<&SceneEventCallbackAdapter::InvokeEntityDestroyed>(this);
}

void SceneEventCallbackAdapter::InvokeEntityCreated(entt::registry &registry, entt::entity entity)
{
  // 构造Entity对象
  Entity userEntity(m_Registry->m_Scene, entity);

  // 触发回调
  m_CreateEntityCallback(userEntity);
}

void SceneEventCallbackAdapter::InvokeEntityDestroyed(entt::registry &registry,
                                                      entt::entity entity)
{
  // 构造Entity对象
  Entity userEntity(m_Registry->m_Scene, entity);

  // 触发回调
  m_DestroyEntityCallback(userEntity);
}

void SceneEventCallbackAdapter::UnregisterCallbackEntity()
{
  m_Registry->GetUnderlyingRegistry()
      .on_construct<entt::entity>()
      .disconnect<&SceneEventCallbackAdapter::InvokeEntityCreated>(this);
  m_Registry->GetUnderlyingRegistry()
      .on_destroy<entt::entity>()
      .disconnect<&SceneEventCallbackAdapter::InvokeEntityDestroyed>(this);
}

};  // namespace mite
