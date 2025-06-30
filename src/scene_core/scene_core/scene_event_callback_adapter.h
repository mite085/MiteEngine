#ifndef MITE_SCENE_EVENT_CALLBACK_ADAPTER
#define MITE_SCENE_EVENT_CALLBACK_ADAPTER

#include "scene_event.h"
#include "scene_registry.h"

namespace mite {
class ComponentStateCache;
/**
 * @brief 场景事件回调适配器（模板增强版）
 *
 * 目的：
 * 将EnTT的事件转换成自定义事件，并经由EventBus发布
 * 
 * 通过模板方法自动注册各类组件事件回调，避免重复代码
 */
class SceneEventCallbackAdapter : public CallbackAdapter<SceneRegistry *> {
 public:
  explicit SceneEventCallbackAdapter(SceneRegistry *reg);

  ~SceneEventCallbackAdapter() override;
  // 组件回调函数类型
  using ComponentCallback = std::function<void(Entity, Component &)>;
  using ComponentUpdateCallback = std::function<void(Entity, Component &, Component &)>;
  // 1. 实体与组件注册接口 =============================================
  /**
   * @brief 注册所有回调到原始系统
   * @param source 原始系统对象指针
   */
  void RegisterCallbacks(SceneRegistry *source);

  /**
   * @brief 注销所有回调
   */
  void UnregisterCallbacks();

 public:
  /**
   * @brief 注册实体生命周期回调
   */
  void RegisterEntityCallbacks();
  /**
   * @brief 模板方法：注册组件相关回调
   * @tparam Component 组件类型
   *
   * 自动注册该组件的添加/删除/变更三种事件回调
   */
  template<typename T> void RegisterComponentCallbacks()
  {
    // 组件添加事件
    RegisterCallbackComponentConstruct<T>([this](Entity entity, T &component) {
      PostComponentEvent<ComponentAddedEvent<T>, T>(entity, component);
    });

    // 组件删除事件
    RegisterCallbackComponentDestroy<T>([this](Entity entity, T &component) {
      PostComponentEvent<ComponentRemovedEvent<T>, T>(entity, component);
    });

    // 组件变更事件
    RegisterCallbackComponentUpdate<T>([this](Entity entity, T &component, T &oldComponent) {
      PostComponentEvent<ComponentChangedEvent<T>, T>(entity, component, oldComponent);
    });
  }

  /**
   * @brief 注销Component回调函数
   */
  template<typename T> void UnregisterComponentCallbacks()
  {
    m_Registry->GetUnderlyingRegistry()
        .on_update<T>()
        .template disconnect<&SceneEventCallbackAdapter::InvokeUpdate<T>>(this);
    m_Registry->GetUnderlyingRegistry()
        .on_update<T>()
        .template disconnect<&SceneEventCallbackAdapter::InvokeUpdate<T>>(this);
    m_Registry->GetUnderlyingRegistry()
        .on_destroy<T>()
        .template disconnect<&SceneEventCallbackAdapter::InvokeDestroy<T>>(this);
  }

 private:
  SceneRegistry *m_Registry = nullptr;  // RegisterCallbacks时更新的注册对象

  // 2. 组件事件回调相关(由ComponentSystem全权管理) ===========================
 private:
  /**
   * @brief 
   * @tparam E_T EventType事件类型
   * @tparam T Component组件类型
   * @param entity 
   * @param component 
   */
  template<typename E_T, typename T> void PostComponentEvent(Entity entity, T &component)
  {
    E_T event(entity, component);
    EventBus::Get().Post(event);
  }
  template<typename E_T, typename T>
  void PostComponentEvent(Entity entity, T &component, T &oldComponent)
  {
    // ComponentChangedEvent专用（或许没必要封装成函数）
    E_T event(entity, component, oldComponent);
    EventBus::Get().Post(event);
  }
  /**
   * @brief 注册组件构造回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T>
  void RegisterCallbackComponentConstruct(std::function<void(Entity, T &)> callback)
  {
    // 编译时检查，确保注册使用的模板为Component的子类
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    // 根据typeid构建key-value对，存入哈希表
    const std::type_index type = typeid(T);
    m_ConstructCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };

    // 连接到EnTT的回调系统
    m_Registry->GetUnderlyingRegistry()
        .on_construct<T>()
        .connect<&SceneEventCallbackAdapter::InvokeConstruct<T>>(this);
  }

  /**
   * @brief 注册组件更新回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T>
  void RegisterCallbackComponentUpdate(std::function<void(Entity, T &, T &)> callback)
  {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    const std::type_index type = typeid(T);
    m_UpdateCallbacks[type] = [callback](Entity entity, Component &comp, Component &oldComp) {
      callback(entity, static_cast<T &>(comp), static_cast<T &>(oldComp));
    };

    // 连接到EnTT的回调系统
    m_Registry->GetUnderlyingRegistry()
        .on_update<T>()
        .template connect<&SceneEventCallbackAdapter::InvokeUpdate<T>>(this);
  }

  /**
   * @brief 注册组件销毁回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T>
  void RegisterCallbackComponentDestroy(std::function<void(Entity, T &)> callback)
  {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    const std::type_index type = typeid(T);
    m_DestroyCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };

    // 连接到EnTT的回调系统
    m_Registry->GetUnderlyingRegistry()
        .on_destroy<T>()
        .template connect<&SceneEventCallbackAdapter::InvokeDestroy<T>>(this);
  }

 private:
  /**
   * @brief EnTT原生on_construct事件回调（组件专用版）
   *
   * 注意：on_construct的签名必须匹配void(entt::registry&, entt::entity)
   */
  template<typename T> void InvokeConstruct(entt::registry &reg, entt::entity ent)
  {
    Entity entity{m_Registry->m_Scene, ent};
    T &component = reg.get<T>(ent);
    // 以typeid作为key查表
    const std::type_index type = typeid(T);
    if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
      // 此处将会触发
      // PostComponentEvent<ComponentAddedEvent<T>, T>(entity, component)
      // 的运行，发布ComponentAddedEvent事件。
      it->second(entity, component);
    }

    // 构造时，缓存初始状态
    m_ComponentStateCache.Capture<T>(reg, ent);
  }

  /**
   * @brief EnTT原生on_update事件回调（组件专用版）
   *
   * 注意：on_update的签名必须匹配void(entt::registry&, entt::entity)
   */
  template<typename T> void InvokeUpdate(entt::registry &reg, entt::entity ent)
  {
    Entity entity{m_Registry->m_Scene, ent};
    T &component = reg.get<T>(ent);

    // 需要检查旧Component是否为空
    auto oldComponentPtr = m_ComponentStateCache.GetOldState<T>(ent);
    if (oldComponentPtr != nullptr) {
      // 不空才能安全解引用
      T &oldComponent = *const_cast<T *>(oldComponentPtr);
      const std::type_index type = typeid(T);
      if (auto it = m_UpdateCallbacks.find(type); it != m_UpdateCallbacks.end()) {
        // 此处将会触发
        // PostComponentEvent<ComponentChangedEvent<T>, T>(entity, component);
        // 的运行，发布ComponentChangedEvent事件。
        it->second(entity, component, oldComponent);
      }
    }
    else {
      // 如果空则顺势执行ConstructCallbacks
      const std::type_index type = typeid(T);
      if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
        it->second(entity, component);
      }
    }
    // 完成update，更新缓存
    m_ComponentStateCache.Capture<T>(reg, ent);
  }

  /**
   * @brief EnTT原生on_destroy事件回调（组件专用版）
   *
   * 注意：on_destroy的签名必须匹配void(entt::registry&, entt::entity)
   */
  template<typename T> void InvokeDestroy(entt::registry &reg, entt::entity ent)
  {
    Entity entity{m_Registry->m_Scene, ent};
    T &component = reg.get<T>(ent);
    const std::type_index type = typeid(T);
    if (auto it = m_DestroyCallbacks.find(type); it != m_DestroyCallbacks.end()) {
      // 此处将会触发
      // PostComponentEvent<ComponentRemovedEvent<T>, T>(entity, component);
      // 的运行，发布ComponentRemovedEvent事件。
      it->second(entity, component);
    }
  }



  // 存储所有组件类型的回调(同一组件类型仅存放一个回调函数)
  //
  // 存储内容：不同模板T下的
  // [this](Entity entity, T &component) {
  //    PostComponentEvent<ComponentAddedEvent<T>, T>(entity, component);}
  //
  // 作用：
  // 当Invoke函数触发，准备发布事件时，提供查表操作。
  std::unordered_map<std::type_index, ComponentCallback> m_ConstructCallbacks;
  std::unordered_map<std::type_index, ComponentUpdateCallback> m_UpdateCallbacks;
  std::unordered_map<std::type_index, ComponentCallback> m_DestroyCallbacks;

  // ComponentUpdate专用的组件状态缓存
  ComponentStateCache m_ComponentStateCache;

  // 3. 实体事件回调相关(由SceneObserver全权管理) ===================================
 private:
  // 实体生命周期回调类型
  using EntityCallback = std::function<void(Entity)>;
  // 带优先级的回调包装器
  struct EntityCallbackWrapper {
    EntityCallback callback;
    size_t id = 0;     // 唯一标识
  };

  /**
   * @brief 注册实体创建回调
   * @param callback 回调函数
   * @param priority 调用优先级（数值越大越早执行）
   * @return 可用于取消注册的回调ID
   */
  void RegisterCallbackEntityCreated(EntityCallback callback);

  /**
   * @brief 注册实体销毁回调（在实体实际销毁时调用）
   * @param callback 回调函数
   * @param priority 调用优先级（数值越大越早执行）
   * @return 回调ID
   */
  void RegisterCallbackEntityDestroyed(EntityCallback callback);


  /**
   * @brief EnTT原生on_construct事件回调（实体专用版）
   *
   * 注意：on_construct的签名必须匹配void(entt::registry&, entt::entity)
   */
  void InvokeEntityCreated(entt::registry &registry, entt::entity entity);

  /**
   * @brief EnTT原生on_destroy事件回调（实体专用版）
   *
   * 注意：on_destroy的签名必须匹配void(entt::registry&, entt::entity)
   */
  void InvokeEntityDestroyed(entt::registry &registry, entt::entity entity);

  /**
   * @brief 注销全部Entity回调函数
   */
  void UnregisterCallbackEntity();

 private:
  // 回调存储结构
  EntityCallback m_CreateEntityCallback;
  EntityCallback m_DestroyEntityCallback;

  size_t m_NextEntityCallbackID = 1;  // 全局的CallBack自增计数器，同时作为ID

};

/**
 * @brief ComponentUpdate专用的,非模板化的组件状态缓存管理器
 *
 * EnTT原生的on_update仅支持查询更新后的newComponent，所以
 * 需要一个缓存已构建的<entity, component>列表，以确保在发布
 * ComponentChangedEvent事件时，能查询到oldComponent。
 *
 */
class ComponentStateCache {
 public:
  ~ComponentStateCache()
  {
    // 清理所有缓存
    for (auto &pair : m_Caches) {
      delete pair.second;
    }
  }

  /**
   * @brief 捕获指定类型组件的当前状态
   */
  template<typename T> void Capture(entt::registry &reg, entt::entity entity)
  {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    const std::type_index type = typeid(T);
    auto it = m_Caches.find(type);
    if (it == m_Caches.end()) {
      // 为新类型创建缓存
      auto *cache = new TypedCache<T>();
      m_Caches[type] = cache;
      it = m_Caches.find(type);
    }

    // 捕获状态
    static_cast<TypedCache<T> *>(it->second)->Capture(reg, entity);
  }

  /**
   * @brief 获取指定类型组件的旧状态
   */
  template<typename T> const T *GetOldState(entt::entity entity) const
  {
    const std::type_index type = typeid(T);
    auto it = m_Caches.find(type);
    if (it != m_Caches.end()) {
      return static_cast<const TypedCache<T> *>(it->second)->GetOldState(entity);
    }
    return nullptr;
  }

  /**
   * @brief 清除指定实体的缓存
   */
  void Clear(entt::entity entity)
  {
    for (auto &pair : m_Caches) {
      pair.second->Clear(entity);
    }
  }

 private:
  // 类型擦除基类
  struct CacheBase {
    virtual ~CacheBase() = default;
    virtual void Clear(entt::entity entity) = 0;
  };

  // 具体类型的缓存实现
  template<typename T> struct TypedCache : CacheBase {
    std::unordered_map<entt::entity, T *> cache;

    void Capture(entt::registry &reg, entt::entity entity)
    {
      if (reg.valid(entity) && reg.all_of<T>(entity)) {
        cache[entity] = &reg.get<T>(entity);
      }
    }

    const T *GetOldState(entt::entity entity) const
    {
      auto it = cache.find(entity);
      return it != cache.end() ? it->second : nullptr;
    }

    void Clear(entt::entity entity) override
    {
      // 注意：
      // Clear在entity销毁时触发，
      // 此entity未必注册所有component，
      // 所以cache有可能不包含本entity。
      if (cache.find(entity) != cache.end())
        cache.erase(entity);
    }
  };

  std::unordered_map<std::type_index, CacheBase *> m_Caches;
};

};  // namespace mite

#endif
