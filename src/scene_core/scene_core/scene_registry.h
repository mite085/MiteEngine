#ifndef MITE_SCENE_REGISTRY
#define MITE_SCENE_REGISTRY

#include "entity.h"
#include "component.h"

namespace mite {
// 前向声明
class Scene;
/**
 * @brief 对entt::registry的安全封装
 *
 * 提供类型安全的实体和组件操作，避免直接暴露entt::entity
 */
class SceneRegistry {
 public:
  SceneRegistry(std::weak_ptr<Scene> scene);
  ~SceneRegistry();

  // 1. 实体管理 ===================================================
 public:
  /**
   * @brief 创建新实体
   * @param name 新实体的名字
   * @return 新创建的实体
   */
  Entity CreateEntity(const std::string name = "");

  /**
   * @brief 销毁实体
   * @param entity 要销毁的实体
   */
  void DestroyEntity(Entity entity);

  /**
   * @brief 检查实体是否有效
   * @param entity 要检查的实体
   * @return 是否有效
   */
  bool IsValid(Entity entity) const;

  /**
   * @brief 清空注册表
   */
  void Clear();

  // 2. 组件操作 - 基础 ============================================
 public:
  /**
   * @brief 添加组件（构造新组件）
   * @tparam T 组件类型
   * @tparam Args 构造参数类型
   * @param entity 目标实体
   * @param args 组件构造参数
   * @return 新添加的组件引用
   */
  template<typename T, typename... Args> T &AddComponent(Entity entity, Args &&...args)
  {
    // 使用Assert断言，确保entity有效
    assert(IsValid(entity));

    // 若该实体已经挂载了和当前添加组件
    // 同类型的组件，则移除原组件，
    // 以确保新组件正常添加
    if (HasComponent<T>(entity)) {
      RemoveComponent<T>(entity);
    }

    // 注意：若T已经被RegisterCallbackComponentConstruct注册，
    // 这里会触发entt内部的回调，运行被注册的callback函数
    T &component = m_Registry.emplace<T>(entity.GetHandle(), std::forward<Args>(args)...);
    component.SetOwnerEntity(entity);
    return component;
  }

  /**
   * @brief 获取或添加组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件引用
   */
  template<typename T> T &GetOrAddComponent(Entity entity)
  {
    assert(IsValid(entity));

    // 破坏性低于AddComponent的用法，
    // 原组件存在时不移除，直接返回原组件
    T &component = m_Registry.get_or_emplace<T>(entity.GetHandle());
    component.SetOwnerEntity(entity);
    return component;
  }

  /**
   * @brief 移除组件
   * @tparam T 组件类型
   * @param entity 目标实体
   */
  template<typename T> void RemoveComponent(Entity entity)
  {
    // 销毁组件时，无需使用assert断言确保entity有效。
    if (IsValid(entity) && m_Registry.all_of<T>(entity.GetHandle())) {

      // 注意：若T已经被RegisterCallbackComponentDestroy注册，
      // 这里会触发回调，运行被注册的callback函数
      m_Registry.remove<T>(entity.GetHandle());
    }
  }

  /**
   * @brief 替换组件
   * @param entity 实体对象
   * @param ...args 新组件的构造函数参数包
   * @return 新组件的引用
   */
  template<typename T, typename... Args> T &ReplaceComponent(Entity entity, Args &&...args)
  {
    // 获取旧组件
    T &oldComponent = GetComponent<T>(entity);

    // 执行实际替换
    T &newComponent = m_Registry.replace<T>(entity, std::forward<Args>(args)...);

    // 发布变更事件（包含新旧组件）
    EventBus::Get().Post(ComponentChangedEvent<T>(entity, newComponent, oldComponent));

    return newComponent;
  }

  /**
   * @brief 组件的部分更新
   * @param entity 实体对象
   * @param ...args 新组件的构造函数参数包
   * @return 新组件的引用
   */
  template<typename T, typename... Args> T &PatchComponent(Entity entity, Args &&...args)
  {
    // 获取旧组件
    T &oldComponent = GetComponent<T>(entity);

    // 执行部分更新
    T &component = m_Registry.patch<T>(entity, std::forward<Args>(args)...);

    // 发布变更事件（包含新旧组件）
    EventBus::Get().Post(ComponentChangedEvent<T>(entity, component, oldComponent));

    return component;
  }

  /**
   * @brief 检查实体是否拥有组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 是否拥有该组件
   */
  template<typename... T> bool HasComponent(Entity entity) const
  {
    return IsValid(entity) && m_Registry.all_of<T...>(entity.GetHandle());
  }

  // 3. 组件操作 - 获取 ============================================
 public:
  /**
   * @brief 获取组件（非const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> T &GetComponent(Entity entity)
  {
    // 使用Assert断言，确保entity具有该组件。
    // 若无法确定，则应当使用TryGetComponent
    assert(HasComponent<T>(entity));
    return m_Registry.get<T>(entity.GetHandle());
  }

  /**
   * @brief 获取组件（const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件const引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> const T &GetComponent(Entity entity) const
  {
    assert(HasComponent<T>(entity));
    return m_Registry.get<T>(entity.GetHandle());
  }

  /**
   * @brief 尝试获取组件（非const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件指针（不存在时返回nullptr）
   */
  template<typename T> T *TryGetComponent(Entity entity)
  {
    if (!IsValid(entity))
      return nullptr;
    return m_Registry.try_get<T>(entity.GetHandle());
  }

  /**
   * @brief 尝试获取组件（const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件const指针（不存在时返回nullptr）
   */
  template<typename T> const T *TryGetComponent(Entity entity) const
  {
    if (!IsValid(entity))
      return nullptr;
    return m_Registry.try_get<T>(entity.GetHandle());
  }

  // 4. 视图和查询 =================================================
 public:
  /**
   * @brief 检查实体是否拥有任何指定的组件
   * @tparam Component 要检查的组件类型(支持多个组件)
   * @param entity 目标实体
   * @return 是否拥有任意一个指定组件
   *
   * 使用示例: 检查某个entity是否具有变换组件、可见性组件和层次结构组件中的任意一种
   * if(m_Registry.AnyOf<TransformComponent, VisibilityComponent, HierarchyComponent>(entity))
   */
  template<typename... Component> bool AnyOf(Entity entity) const
  {
    return IsValid(entity) && m_Registry.any_of<Component...>(entity.GetHandle());
  }

  /**
   * @brief 检查实体是否拥有所有指定的组件
   * @tparam Component 要检查的组件类型(支持多个组件)
   * @param entity 目标实体
   * @return 是否拥有所有指定组件
   *
   * 使用示例: 检查某个entity是否同时具有变换组件、可见性组件和层次结构组件
   * if(m_Registry.AllOf<TransformComponent, VisibilityComponent, HierarchyComponent>(entity))
   */
  template<typename... Component> bool AllOf(Entity entity) const
  {
    return IsValid(entity) && m_Registry.all_of<Component...>(entity.GetHandle());
  }

  /**
   * @brief 获取当前registry中所有有效的Entity集合
   * @return 包含所有有效Entity的vector（按创建顺序）
   * 
   * 这个函数应当是const成员函数，但entt::registry
   * 的storge未提供const方法，所以无法置为const
   */
  std::vector<Entity> GetAllEntities();

  /**
   * @brief 获取当前registry中所有拥有指定组件的Entity集合
   * @tparam Component 要筛选的组件类型
   * @tparam Exclude 要从视图中排除的组件类型
   * @return 包含所有符合条件的Entity的vector（按创建顺序）
   *
   * 使用示例：获取所有同时具备变换组件、可见性组件的实体
   * m_Registry.GetEntitiesWith<TransformComponent, VisibilityComponent>()
   */
  template<typename... Component> std::vector<Entity> GetEntitiesWith()
  {
    std::vector<Entity> entities;

    // 使用entt::registry::view方法，
    // 获取符合类型要求的Component列表
    auto view = m_Registry.view<Component...>();

    for (auto entity : view) {
      if (m_Registry.valid(entity)) {
        entities.emplace_back(m_Scene, entity);
      }
    }

    return entities;
  }

  // 5. 原生访问（禁止外部调用） ========================================
 private:
  /**
   * @brief 获取底层registry（谨慎使用）
   * @return 底层entt::registry引用
   */
  entt::registry &GetUnderlyingRegistry()
  {
    return m_Registry;
  }

  /**
   * @brief 获取底层registry（const版本，谨慎使用）
   * @return 底层entt::registry const引用
   */
  const entt::registry &GetUnderlyingRegistry() const
  {
    return m_Registry;
  }



  // 8. 基本数据存储 ===============================================
 private:
  entt::registry m_Registry;     // 底层EnTT registry
  std::weak_ptr<Scene> m_Scene;  // 场景引用

  // 将SceneEventCallbackAdapter设为友元，允许直接访问底层registry（用于注册回调）
  friend class SceneEventCallbackAdapter;
};
};  // namespace mite

#endif
