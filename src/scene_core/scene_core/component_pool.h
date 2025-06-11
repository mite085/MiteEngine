#ifndef MITE_SCENE_COMPONENT_POOL
#define MITE_SCENE_COMPONENT_POOL

#include "entity.h"

namespace mite {
/**
 * @class IComponentPool
 * @brief 组件池的抽象基接口
 *
 * 提供不依赖具体组件类型的通用操作接口，
 * 允许ComponentManager以统一方式管理所有类型的组件池
 */
class IComponentPool {
 public:
  virtual ~IComponentPool() = default;

  /**
   * @brief 移除指定实体的组件
   * @param entity 目标实体
   */
  virtual void Remove(Entity entity) = 0;

  /**
   * @brief 检查实体是否拥有组件
   * @param entity 目标实体
   * @return 如果存在返回true
   */
  virtual bool Has(Entity entity) const = 0;

  /**
   * @brief 获取所有拥有该组件的实体列表
   * @return 实体列表
   */
  virtual std::vector<Entity> GetEntities() const = 0;

  /**
   * @brief 获取组件数量
   * @return 当前存储的组件总数
   */
  virtual size_t Size() const = 0;
};

/**
 * @class ComponentPool
 * @brief 特定类型组件的具体存储实现
 * @tparam T 组件类型，必须继承自Component
 *
 * 采用SOA(Structure of Arrays)存储模式优化缓存效率：
 * 1. 实体ID和组件数据分开存储
 * 2. 相同索引的实体和组件对应
 * 3. 使用哈希表加速实体查找
 */
template<typename T> class ComponentPool : public IComponentPool {
 public:
  /**
   * @brief 添加组件到实体
   * @param entity 目标实体
   * @return 新组件的引用
   * @note 如果实体已有该组件则抛出异常
   */
  T &Add(Entity entity);

  /**
   * @brief 移除实体的组件(override)
   */
  void Remove(Entity entity) override;

  /**
   * @brief 检查实体是否拥有组件(override)
   */
  bool Has(Entity entity) const override;

  /**
   * @brief 获取实体的组件
   * @param entity 目标实体
   * @return 组件引用
   * @throws std::out_of_range 如果组件不存在
   */
  T &Get(Entity entity);

  /**
   * @brief 获取实体的组件(常量版本)
   */
  const T &Get(Entity entity) const;

  /**
   * @brief 获取所有实体列表(override)
   */
  std::vector<Entity> GetEntities() const override;

  /**
   * @brief 获取组件数量(override)
   */
  size_t Size() const override;

 private:
  // Structure of Arrays (SOA)存储方式
  std::vector<Entity> m_Entities;	// 实体列表(与m_Components一一对应)
  std::vector<T> m_Components;		// 组件数据存储(连续内存)

  // 实体到索引的快速映射
  std::unordered_map<Entity, size_t> m_EntityToIndex;
};
};

#endif
