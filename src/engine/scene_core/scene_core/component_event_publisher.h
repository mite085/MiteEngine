#ifndef MITE_SCENE_CORE_EVENT_CALLBACK_ADAPTER
#define MITE_SCENE_CORE_EVENT_CALLBACK_ADAPTER

#include "component.h"
#include "entity.h"

namespace mite {
/**
 * @brief 组件事件生产者
 *
 * 功能：负责组件创建/销毁事件的生产（仅负责创建/销毁，自定义事件无关）
 */
class ComponentEventPublisher {
 public:
  explicit ComponentEventPublisher();
  ~ComponentEventPublisher();

  // 组件回调函数类型
  using ComponentCallback = std::function<void(Entity, Component &)>;

  /**
   * @brief 注销所有回调
   */
  void UnregisterCallbacks();

  /**
   * @brief 注册组件相关回调
   */
  template <typename T>
  void RegisterComponentCallbacks() {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must inherit from Component");

    // 组件添加事件
    RegisterCallbackComponentConstruct<T>([](Entity entity, T &component) {
      EventBus::Publish<ComponentAddedEvent<T>>(entity, component);
    });

    // 组件删除事件
    RegisterCallbackComponentDestroy<T>([](Entity entity, T &component) {
      EventBus::Publish<ComponentRemovedEvent<T>>(entity, component);
    });
  }

  /**
   * @brief 注销组件回调函数
   */
  template <typename T>
  void UnregisterComponentCallbacks() {
    std::unique_lock lock(m_Mutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks.erase(type);
    // m_UpdateCallbacks.erase(type);
    m_DestroyCallbacks.erase(type);
  }

  // 以下方法由SceneRegistry调用以触发事件
  template <typename T>
  void OnComponentConstructed(Entity entity, Component &component) {
    std::shared_lock lock(m_Mutex);
    const std::type_index type = typeid(component);

    // 触发构造回调
    if (auto it = m_ConstructCallbacks.find(type);
        it != m_ConstructCallbacks.end()) {
      it->second(entity, component);
    }
  }

  template <typename T>
  void OnComponentDestroyed(Entity entity, Component &component) {
    std::shared_lock lock(m_Mutex);
    const std::type_index type = typeid(component);

    // 触发销毁回调
    if (auto it = m_DestroyCallbacks.find(type);
        it != m_DestroyCallbacks.end()) {
      it->second(entity, component);
    }
  }

 private:
  template <typename T>
  void RegisterCallbackComponentConstruct(
      std::function<void(Entity, T &)> callback) {
    std::unique_lock lock(m_Mutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };
  }

  template <typename T>
  void RegisterCallbackComponentDestroy(
      std::function<void(Entity, T &)> callback) {
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
