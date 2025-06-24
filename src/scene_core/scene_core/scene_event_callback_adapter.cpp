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

  // 此处仅注册实体回调，组件回调由ComponentSystemManager的
  // RegisterSystem方法负责注册。
  //
  // 好处是：
  // 无论后续添加多少新的Component类型,
  // 均只受Manager管理，无需在此处添加新的注册行为。
  //
  // 而坏处是：
  // 多了一层ComponentSystemManager对Adapter的依赖，
  // 需要在初始化时注意两者创建顺序，
  // 且Adapter的构造与m_Registry的获取绑定。
  RegisterEntityCallbacks();
}

void SceneEventCallbackAdapter::UnregisterCallbacks()
{
  UnregisterCallbackComponent();
  UnregisterCallbackEntity();
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

// 2. 组件事件回调相关 ===============================================

void SceneEventCallbackAdapter::UnregisterCallbackComponent()
{
  m_ConstructCallbacks.clear();
  m_UpdateCallbacks.clear();
  m_DestroyCallbacks.clear();
}

// 3. 实体事件回调相关 ===============================================

size_t SceneEventCallbackAdapter::RegisterCallbackEntityCreated(EntityCallback callback)
{
  // 连接到EnTT的回调系统
  m_Registry->GetUnderlyingRegistry()
      .on_construct<entt::entity>()
      .connect<&SceneEventCallbackAdapter::InvokeEntityCreated>(
      this);

  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.createdCallbacks.push_back({std::move(callback), id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.createdCallbacks;
  return id;
}

size_t SceneEventCallbackAdapter::RegisterCallbackEntityDestroyed(EntityCallback callback)
{
  // 连接到EnTT的回调系统
  m_Registry->GetUnderlyingRegistry()
      .on_destroy<entt::entity>()
      .connect<&SceneEventCallbackAdapter::InvokeEntityDestroyed>(this);
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.destroyCallbacks.push_back({std::move(callback), id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.destroyCallbacks;
  return id;
}

void SceneEventCallbackAdapter::InvokeEntityCreated(entt::registry &registry, entt::entity entity)
{
  // 构造Entity对象
  Entity userEntity(m_Registry->m_Scene, entity);

  // 按顺序触发回调
  for (auto wrapper : m_EntityCallbacks.createdCallbacks) {
    wrapper.callback(userEntity);
  }
}

void SceneEventCallbackAdapter::InvokeEntityDestroyed(entt::registry &registry,
                                                      entt::entity entity)
{
  // 构造Entity对象
  Entity userEntity(m_Registry->m_Scene, entity);

  // 按顺序触发回调
  for (auto wrapper : m_EntityCallbacks.destroyCallbacks) {
    wrapper.callback(userEntity);
  }
}

void SceneEventCallbackAdapter::UnregisterCallbackEntity(size_t callbackId)
{
  auto it = m_EntityCallbacks.entityCallbackMap.find(callbackId);
  if (it != m_EntityCallbacks.entityCallbackMap.end()) {
    auto &callbacks = *it->second;
    // 根据remove_if返回的迭代器，
    // 从callbacks列表中，
    // 移除所有id等于callbackId的回调函数。
    callbacks.erase(
        std::remove_if(callbacks.begin(),
                       callbacks.end(),
                       [callbackId](const auto &wrapper) { return wrapper.id == callbackId; }),
        callbacks.end());
    m_EntityCallbacks.entityCallbackMap.erase(it);
  }
}

void SceneEventCallbackAdapter::UnregisterCallbackEntity()
{
  m_EntityCallbacks.createdCallbacks.clear();
  m_EntityCallbacks.destroyCallbacks.clear();
  m_EntityCallbacks.entityCallbackMap.clear();
}

};  // namespace mite
