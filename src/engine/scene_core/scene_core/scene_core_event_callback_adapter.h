#ifndef MITE_SCENE_CORE_EVENT_CALLBACK_ADAPTER
#define MITE_SCENE_CORE_EVENT_CALLBACK_ADAPTER

#include "scene_core_event.h"

namespace mite {
/**
 * @brief 场景事件回调适配器
 */
class SceneCoreEventCallbackAdapter {
 public:
  explicit SceneCoreEventCallbackAdapter();
  ~SceneCoreEventCallbackAdapter();

  // 组件回调函数类型
  using ComponentCallback = std::function<void(Entity, Component &)>;
  // using ComponentUpdateCallback = std::function<void(Entity, Component &, Component &)>;

  /**
   * @brief 注册所有回调
   */
  void RegisterCallbacks();

  /**
   * @brief 注销所有回调
   */
  void UnregisterCallbacks();

  /**
   * @brief 注册组件相关回调
   */
  template<typename T> void RegisterComponentCallbacks()
  {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    // 组件添加事件
    RegisterCallbackComponentConstruct<T>([this](Entity entity, T &component) {
      ComponentAddedEvent<T> event(entity, component);
      EventBus::Get().Post<ComponentAddedEvent<T>>(event);
    });

    // 组件删除事件
    RegisterCallbackComponentDestroy<T>([this](Entity entity, T &component) {
      ComponentRemovedEvent<T> event(entity, component);
      EventBus::Get().Post<ComponentRemovedEvent<T>>(event);
    });
  }

  /**
   * @brief 注销组件回调函数
   */
  template<typename T> void UnregisterComponentCallbacks()
  {
    std::unique_lock lock(m_Mutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks.erase(type);
    // m_UpdateCallbacks.erase(type);
    m_DestroyCallbacks.erase(type);
  }

  // 以下方法由SceneRegistry调用以触发事件
  template<typename T> void OnComponentConstructed(Entity entity, Component &component)
  {
    std::shared_lock lock(m_Mutex);
    const std::type_index type = typeid(component);

    // 触发构造回调
    if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
      it->second(entity, component);
    }

  }

  template<typename T> void OnComponentDestroyed(Entity entity, Component &component)
  {
    std::shared_lock lock(m_Mutex);
    const std::type_index type = typeid(component);

    // 触发销毁回调
    if (auto it = m_DestroyCallbacks.find(type); it != m_DestroyCallbacks.end()) {
      it->second(entity, component);
    }
  }

 private:

  template<typename T>
  void RegisterCallbackComponentConstruct(std::function<void(Entity, T &)> callback)
  {
    std::unique_lock lock(m_Mutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };
  }

  template<typename T>
  void RegisterCallbackComponentDestroy(std::function<void(Entity, T &)> callback)
  {
    std::unique_lock lock(m_Mutex);
    const std::type_index type = typeid(T);
    m_DestroyCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };
  }

 private:
  mutable std::shared_mutex m_Mutex;

  std::unordered_map<std::type_index, ComponentCallback> m_ConstructCallbacks;
  std::unordered_map<std::type_index, ComponentCallback> m_DestroyCallbacks;
};
}  // namespace mite
#endif
