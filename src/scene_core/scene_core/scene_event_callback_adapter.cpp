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
  RegisterCallbackEntityPreDestroyed([this](Entity entity) {
    EntityPreDestroyedEvent event(entity);
    EventBus::Get().Post(event);
  });
  RegisterCallbackEntityPostDestroyed([this](Entity entity) {
    EntityPostDestroyedEvent event(entity);
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

size_t SceneEventCallbackAdapter::RegisterCallbackEntityCreated(EntityCallback callback,
                                                                int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.createdCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.createdCallbacks;
  SortCallbackList(m_EntityCallbacks.createdCallbacks);
  return id;
}

size_t SceneEventCallbackAdapter::RegisterCallbackEntityPreDestroyed(EntityCallback callback,
                                                                     int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.preDestroyCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.preDestroyCallbacks;
  SortCallbackList(m_EntityCallbacks.preDestroyCallbacks);
  return id;
}

size_t SceneEventCallbackAdapter::RegisterCallbackEntityPostDestroyed(EntityCallback callback,
                                                                      int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.postDestroyCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.postDestroyCallbacks;
  SortCallbackList(m_EntityCallbacks.postDestroyCallbacks);
  return id;
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
  m_EntityCallbacks.preDestroyCallbacks.clear();
  m_EntityCallbacks.postDestroyCallbacks.clear();
  m_EntityCallbacks.entityCallbackMap.clear();
}

void SceneEventCallbackAdapter::SortCallbackList(std::vector<EntityCallbackWrapper> &callbacks)
{
  std::sort(callbacks.begin(), callbacks.end(), [](const auto &a, const auto &b) {
    return a.priority > b.priority;
  });
}

};  // namespace mite
