#ifndef MITE_SCENE_EVENT_CALLBACK_ADAPTER
#define MITE_SCENE_EVENT_CALLBACK_ADAPTER

#include "scene_event.h"

namespace mite {
///**
// * @brief 组件状态缓存管理器
// */
// class ComponentStateCache {
// public:
//  ~ComponentStateCache()
//  {
//    std::unique_lock lock(m_Mutex);
//    for (auto &pair : m_Caches) {
//      delete pair.second;
//    }
//  }
//
//  /**
//   * @brief 捕获指定类型组件的当前状态
//   */
//  template<typename T> void Capture(Entity entity, T *component)
//  {
//    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
//
//    std::unique_lock lock(m_Mutex);
//    const std::type_index type = typeid(T);
//    auto it = m_Caches.find(type);
//    if (it == m_Caches.end()) {
//      auto *cache = new TypedCache<T>();
//      m_Caches[type] = cache;
//      it = m_Caches.find(type);
//    }
//
//    static_cast<TypedCache<T> *>(it->second)->Capture(entity, component);
//  }
//
//  /**
//   * @brief 获取指定类型组件的旧状态
//   */
//  template<typename T> const T *GetOldState(Entity entity) const
//  {
//    std::shared_lock lock(m_Mutex);
//    const std::type_index type = typeid(T);
//    auto it = m_Caches.find(type);
//    if (it != m_Caches.end()) {
//      return static_cast<const TypedCache<T> *>(it->second)->GetOldState(entity);
//    }
//    return nullptr;
//  }
//
//  /**
//   * @brief 清除指定实体的缓存
//   */
//  void Clear(Entity entity)
//  {
//    std::unique_lock lock(m_Mutex);
//    for (auto &pair : m_Caches) {
//      pair.second->Clear(entity);
//    }
//  }
//
// private:
//  // 类型擦除基类
//  struct CacheBase {
//    virtual ~CacheBase() = default;
//    virtual void Clear(Entity entity) = 0;
//  };
//
//  // 具体类型的缓存实现
//  template<typename T> struct TypedCache : CacheBase {
//    std::unordered_map<Entity, T *> cache;
//
//    void Capture(Entity entity, T *component)
//    {
//      if (entity.IsValid() && component != nullptr) {
//        cache[entity] = component;
//      }
//    }
//
//    const T *GetOldState(Entity entity) const
//    {
//      auto it = cache.find(entity);
//      return it != cache.end() ? it->second : nullptr;
//    }
//
//    void Clear(Entity entity) override
//    {
//      cache.erase(entity);
//    }
//  };
//
//  mutable std::shared_mutex m_Mutex;
//  std::unordered_map<std::type_index, CacheBase *> m_Caches;
//};

/**
 * @brief 场景事件回调适配器
 *
 * TODO：
 * 原则上应当继承自public CallbackAdapter<SceneRegistry*>
 * 但现在RegisterCallbacks()由SceneRegistry触发，
 * 所以此处依赖关系修改为SceneRegistry依赖Adapter，
 * 所以无法使用原先的继承模式
 *
 * TODO:
 * 确认ComponentChangedEvent是否有必要
 */
class SceneEventCallbackAdapter {
 public:
  explicit SceneEventCallbackAdapter();
  ~SceneEventCallbackAdapter();

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
      EventBus::Get().Post(event);
    });

    // 组件删除事件
    RegisterCallbackComponentDestroy<T>([this](Entity entity, T &component) {
      ComponentRemovedEvent<T> event(entity, component);
      EventBus::Get().Post(event);
    });

    //// 组件变更事件
    // RegisterCallbackComponentUpdate<T>([this](Entity entity, T &component, T &oldComponent) {
    //   PostComponentEvent<ComponentChangedEvent<T>, T>(entity, component, oldComponent);
    // });
  }

  /**
   * @brief 注销组件回调函数
   */
  template<typename T> void UnregisterComponentCallbacks()
  {
    std::unique_lock lock(m_CallbackMutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks.erase(type);
    // m_UpdateCallbacks.erase(type);
    m_DestroyCallbacks.erase(type);
  }

  // 以下方法由SceneRegistry调用以触发事件
  template<typename T> void OnComponentConstructed(Entity entity, Component &component)
  {
    std::shared_lock lock(m_CallbackMutex);
    const std::type_index type = typeid(component);

    // 触发构造回调
    if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
      it->second(entity, component);
    }

    //// 缓存初始状态
    // m_ComponentStateCache.Capture(entity, &component);
  }
  // template<typename T> void OnComponentUpdated(Entity entity, Component &component)
  //{
  //   std::shared_lock lock(m_CallbackMutex);
  //   const std::type_index type = typeid(component);

  //  // 获取旧状态
  //  auto oldComponentPtr = m_ComponentStateCache.GetOldState(entity);

  //  if (oldComponentPtr != nullptr) {
  //    // 触发更新回调
  //    if (auto it = m_UpdateCallbacks.find(type); it != m_UpdateCallbacks.end()) {
  //      it->second(entity, component, *oldComponentPtr);
  //    }
  //  }
  //  else {
  //    // 如果没有旧状态，则触发构造回调
  //    if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
  //      it->second(entity, component);
  //    }
  //  }

  //  // 更新缓存
  //  m_ComponentStateCache.Capture(type, entity, &component);
  //}
  template<typename T> void OnComponentDestroyed(Entity entity, Component &component)
  {
    std::shared_lock lock(m_CallbackMutex);
    const std::type_index type = typeid(component);

    // 触发销毁回调
    if (auto it = m_DestroyCallbacks.find(type); it != m_DestroyCallbacks.end()) {
      it->second(entity, component);
    }

    //// 清除缓存
    // m_ComponentStateCache.Clear(entity);
  }

 private:
  // template<typename E_T, typename T> void PostComponentEvent(Entity entity, T &component)
  //{
  //   E_T event(entity, component);
  //   EventBus::Get().Post(event);
  // }

  // template<typename E_T, typename T>
  // void PostComponentEvent(Entity entity, T &component, T &oldComponent)
  //{
  //   E_T event(entity, component, oldComponent);
  //   EventBus::Get().Post(event);
  // }

  template<typename T>
  void RegisterCallbackComponentConstruct(std::function<void(Entity, T &)> callback)
  {
    std::unique_lock lock(m_CallbackMutex);
    const std::type_index type = typeid(T);
    m_ConstructCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };
  }

  // template<typename T>
  // void RegisterCallbackComponentUpdate(std::function<void(Entity, T &, T &)> callback)
  //{
  //   std::unique_lock lock(m_CallbackMutex);
  //   const std::type_index type = typeid(T);
  //   m_UpdateCallbacks[type] = [callback](Entity entity, Component &comp, Component &oldComp) {
  //     callback(entity, static_cast<T &>(comp), static_cast<T &>(oldComp));
  //   };
  // }

  template<typename T>
  void RegisterCallbackComponentDestroy(std::function<void(Entity, T &)> callback)
  {
    std::unique_lock lock(m_CallbackMutex);
    const std::type_index type = typeid(T);
    m_DestroyCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };
  }

 private:
  mutable std::shared_mutex m_CallbackMutex;

  std::unordered_map<std::type_index, ComponentCallback> m_ConstructCallbacks;
  // std::unordered_map<std::type_index, ComponentUpdateCallback> m_UpdateCallbacks;
  std::unordered_map<std::type_index, ComponentCallback> m_DestroyCallbacks;

  // ComponentStateCache m_ComponentStateCache;
};
}  // namespace mite
#endif
