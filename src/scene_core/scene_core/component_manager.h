#ifndef MITE_SCENE_COMPONENT_MANAGER
#define MITE_SCENE_COMPONENT_MANAGER

#include "component.h"
#include "component_type.h"
#include "entity.h"

namespace mite {
// 前置声明
class IComponentPool;

/**
 * @class ComponentManager
 * @brief 集中管理所有实体组件的核心管理器
 *
 * 职责：
 * 1. 按类型组织组件存储
 * 2. 提供类型安全的组件访问
 * 3. 管理组件生命周期
 * 4. 支持高效的多组件迭代
 */
class ComponentManager {
 public:
  ComponentManager();
  ~ComponentManager();

  // 禁用拷贝
  ComponentManager(const ComponentManager &) = delete;
  ComponentManager &operator=(const ComponentManager &) = delete;

  /**
   * @brief 向实体添加组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 新组件的引用
   */
  template<typename T> T &AddComponent(Entity entity);

  /**
   * @brief 移除实体的指定类型组件
   * @tparam T 组件类型
   * @param entity 目标实体
   */
  template<typename T> void RemoveComponent(Entity entity);

  /**
   * @brief 检查实体是否拥有指定类型组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 如果存在返回true
   */
  template<typename T> bool HasComponent(Entity entity) const;

  /**
   * @brief 获取实体的指定类型组件
   * @tparam T 组件类型
   * @param entity 目标实体
   * @return 组件引用
   * @throws std::runtime_error 如果组件不存在
   */
  template<typename T> T &GetComponent(Entity entity);

  /**
   * @brief 获取实体的指定类型组件(常量版本)
   */
  template<typename T> const T &GetComponent(Entity entity) const;

  /**
   * @brief 实体销毁时调用，清理相关组件
   * @param entity 被销毁的实体
   */
  void OnEntityDestroyed(Entity entity);

  /**
   * @brief 获取所有注册的组件类型
   * @return 组件类型集合
   */
  std::vector<ComponentType> GetRegisteredComponentTypes() const;

  /**
   * @brief 获取指定类型组件的实体视图
   * @tparam T 组件类型
   * @return 包含该组件的实体列表
   */
  template<typename T> std::vector<Entity> GetEntitiesWith() const;

 private:
  /**
   * @brief 获取或创建特定类型的组件池
   * @tparam T 组件类型
   * @param type 组件类型描述符
   * @return 组件池引用
   */
  template<typename T> IComponentPool &GetComponentPool(ComponentType type);

  // 组件池存储 (类型->池)
  std::unordered_map<ComponentType, std::unique_ptr<IComponentPool>> m_ComponentPools;

  // 实体到组件类型的映射 (实体->类型集合)
  std::unordered_map<Entity, std::vector<ComponentType>> m_EntityComponentMap;
};
}  // namespace mite

#endif