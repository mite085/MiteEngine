#ifndef MITE_SCENE_EVENT_CALLBACK_ADAPTER
#define MITE_SCENE_EVENT_CALLBACK_ADAPTER

#include "scene_event.h"
#include "scene_registry.h"

namespace mite {
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
    RegisterCallbackComponentUpdate<T>([this](Entity entity, T &component) {
      PostComponentEvent<ComponentChangedEvent<T>, T>(entity, component);
    });
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
  void RegisterCallbackComponentUpdate(std::function<void(Entity, T &)> callback)
  {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    const std::type_index type = typeid(T);
    m_UpdateCallbacks[type] = [callback](Entity entity, Component &comp) {
      callback(entity, static_cast<T &>(comp));
    };

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

    m_Registry->GetUnderlyingRegistry()
        .on_destroy<T>()
        .template connect<&SceneEventCallbackAdapter::InvokeDestroy<T>>(this);
  }

 private:
  /**
   * @brief 触发组件构造事件(RegisterCallbackComponentConstruct使用)
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
  }

  /**
   * @brief 触发组件更新事件(内部使用)
   *
   * 注意：on_construct的签名必须匹配void(entt::registry&, entt::entity)
   */
  template<typename T> void InvokeUpdate(entt::registry &reg, entt::entity ent)
  {
    Entity entity{m_Registry->m_Scene, ent};
    T &component = reg.get<T>(ent);
    const std::type_index type = typeid(T);
    if (auto it = m_UpdateCallbacks.find(type); it != m_UpdateCallbacks.end()) {
      // 此处将会触发
      // PostComponentEvent<ComponentChangedEvent<T>, T>(entity, component);
      // 的运行，发布ComponentChangedEvent事件。
      it->second(entity, component);
    }
  }

  /**
   * @brief 触发组件销毁事件(内部使用)
   *
   * 注意：on_construct的签名必须匹配void(entt::registry&, entt::entity)
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

  /**
   * @brief 卸载全部Component回调函数
   */
  void UnregisterCallbackComponent();

  // 存储所有组件类型的回调(同一组件类型仅存放一个回调函数)
  //
  // 存储内容：不同模板T下的
  // [this](Entity entity, T &component) {
  //    PostComponentEvent<ComponentAddedEvent<T>, T>(entity, component);}
  //
  // 作用：
  // 当Invoke函数触发，准备发布事件时，提供查表操作。
  std::unordered_map<std::type_index, ComponentCallback> m_ConstructCallbacks;
  std::unordered_map<std::type_index, ComponentCallback> m_UpdateCallbacks;
  std::unordered_map<std::type_index, ComponentCallback> m_DestroyCallbacks;

  // 3. 实体事件回调相关(由SceneObserver全权管理) ===================================
 private:
  // 实体生命周期回调类型
  using EntityCallback = std::function<void(Entity)>;
  // 带优先级的回调包装器
  struct EntityCallbackWrapper {
    EntityCallback callback;
    int priority = 0;  // 默认优先级，数字越大优先级越高
    size_t id = 0;     // 唯一标识
  };

  /**
   * @brief 注册实体创建回调
   * @param callback 回调函数
   * @param priority 调用优先级（数值越大越早执行）
   * @return 可用于取消注册的回调ID
   */
  size_t RegisterCallbackEntityCreated(EntityCallback callback, int priority = 0);

  /**
   * @brief 注册实体销毁回调（在实体实际销毁前调用）
   * @param callback 回调函数
   * @param priority 调用优先级（数值越大越早执行）
   * @return 回调ID
   */
  size_t RegisterCallbackEntityPreDestroyed(EntityCallback callback, int priority = 0);

  /**
   * @brief 注册实体销毁回调（在实体实际销毁后调用）
   * @param callback 回调函数
   * @param priority 调用优先级（数值越大越早执行）
   * @return 回调ID
   */
  size_t RegisterCallbackEntityPostDestroyed(EntityCallback callback, int priority = 0);

  /**
   * @brief 卸载指定的Entity回调函数
   * @param callbackId 由注册函数返回的ID
   */
  void UnregisterCallbackEntity(size_t callbackId);

  /**
   * @brief 卸载全部Entity回调函数
   */
  void UnregisterCallbackEntity();

 private:
  // 回调存储结构
  struct EntityCallbackLists {
    std::vector<EntityCallbackWrapper> createdCallbacks;
    std::vector<EntityCallbackWrapper> preDestroyCallbacks;
    std::vector<EntityCallbackWrapper> postDestroyCallbacks;
    std::unordered_map<size_t, std::vector<EntityCallbackWrapper> *> entityCallbackMap;
  } m_EntityCallbacks;

  size_t m_NextEntityCallbackID = 1;  // 全局的CallBack自增计数器，同时作为ID

  /**
   * @brief 排序回调列表（按优先级降序）
   * @param callbacks 一般为EntityCallbackLists中的一个，如createdCallbacks
   */
  void SortCallbackList(std::vector<EntityCallbackWrapper> &callbacks);
};
};  // namespace mite

#endif
