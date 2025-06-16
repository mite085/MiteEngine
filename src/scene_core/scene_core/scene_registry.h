#ifndef MITE_SCENE_REGISTRY
#define MITE_SCENE_REGISTRY

#include "entity.h"

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

  // 实体管理 ===================================================

  /**
   * @brief 创建新实体
   * @param name 新实体的名字
   * @return 新创建的实体
   */
  Entity CreateEntity(const std::string &name);

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

  // 组件操作 - 基础 ============================================

  /**
   * @brief 添加组件（构造新组件）
   * @tparam T 组件类型
   * @tparam Args 构造参数类型
   * @param entity 目标实体
   * @param args 组件构造参数
   * @return 新添加的组件引用
   */
  template<typename T, typename... Args> T &AddComponent(Entity entity, Args &&...args);

  /**
   * @brief 获取或添加组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件引用
   */
  template<typename T> T &GetOrAddComponent(Entity entity);

  /**
   * @brief 移除组件
   * @tparam T 组件类型
   * @param entity 目标实体
   */
  template<typename T> void RemoveComponent(Entity entity);

  /**
   * @brief 检查实体是否拥有组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 是否拥有该组件
   */
  template<typename T> bool HasComponent(Entity entity) const;

  // 组件操作 - 获取 ============================================

  /**
   * @brief 获取组件（非const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> T &GetComponent(Entity entity);

  /**
   * @brief 获取组件（const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件const引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> const T &GetComponent(Entity entity) const;

  /**
   * @brief 尝试获取组件（非const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件指针（不存在时返回nullptr）
   */
  template<typename T> T *TryGetComponent(Entity entity);

  /**
   * @brief 尝试获取组件（const版本）
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件const指针（不存在时返回nullptr）
   */
  template<typename T> const T *TryGetComponent(Entity entity) const;

  // 视图和查询 =================================================

  /**
   * @brief 检查实体是否拥有任何指定的组件
   * @tparam Component 要检查的组件类型
   * @param entity 目标实体
   * @return 是否拥有任意一个指定组件
   */
  template<typename... Component> bool AnyOf(Entity entity) const;

  /**
   * @brief 检查实体是否拥有所有指定的组件
   * @tparam Component 要检查的组件类型
   * @param entity 目标实体
   * @return 是否拥有所有指定组件
   */
  template<typename... Component> bool AllOf(Entity entity) const;

  /**
   * @brief 获取当前registry中所有有效的Entity集合
   * @return 包含所有有效Entity的vector（按创建顺序）
   */
  std::vector<Entity> GetAllEntities();

  /**
   * @brief 获取当前registry中所有拥有指定组件的Entity集合
   * @tparam Component 要筛选的组件类型
   * @tparam Exclude 要从视图中排除的组件类型
   * @return 包含所有符合条件的Entity的vector（按创建顺序）
   */
  template<typename... Component> std::vector<Entity> GetEntitiesWith();

  // 原生访问（谨慎使用） ========================================

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

  // 事件回调相关 ===============================================

  // 组件构造回调函数类型
  using ComponentConstructCallback = std::function<void(Entity, Component &)>;
  // 组件更新回调函数类型
  using ComponentUpdateCallback = std::function<void(Entity, Component &)>;
  // 组件销毁回调函数类型
  using ComponentDestroyCallback = std::function<void(Entity, Component &)>;

  /**
   * @brief 注册组件构造回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T> void OnComponentConstruct(ComponentConstructCallback callback);

  /**
   * @brief 注册组件更新回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T> void OnComponentUpdate(ComponentUpdateCallback callback);

  /**
   * @brief 注册组件销毁回调
   * @tparam T 组件类型
   * @param callback 回调函数
   */
  template<typename T> void OnComponentDestroy(ComponentDestroyCallback callback);

 private:
  /**
   * @brief 触发组件构造事件(内部使用)
   */
  template<typename T> void InvokeConstruct(Entity entity, T &component);

  /**
   * @brief 触发组件更新事件(内部使用)
   */
  template<typename T> void InvokeUpdate(Entity entity, T &component);

  /**
   * @brief 触发组件销毁事件(内部使用)
   */
  template<typename T> void InvokeDestroy(Entity entity, T &component);

  // 存储所有组件类型的回调
  std::unordered_map<std::type_index, ComponentConstructCallback> m_ConstructCallbacks;
  std::unordered_map<std::type_index, ComponentUpdateCallback> m_UpdateCallbacks;
  std::unordered_map<std::type_index, ComponentDestroyCallback> m_DestroyCallbacks;

 private:
  entt::registry m_Registry;     // 底层EnTT registry
  std::weak_ptr<Scene> m_Scene;  // 场景引用

  // 将Scene设为友元，允许Scene直接访问底层registry（用于性能关键路径）
  friend class Scene;
};
};  // namespace mite

#endif
