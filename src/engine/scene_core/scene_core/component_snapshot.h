#ifndef MITE_SCENE_CORE_COMPONENT_SNAPSHOT
#define MITE_SCENE_CORE_COMPONENT_SNAPSHOT

#include "component.h"
#include "basic_type/snapshot.h"
#include "scene_core_event.h"

namespace mite {
/**
 * @brief 组件快照模板类
 *
 * 负责存储和管理特定类型组件的数据快照，通过事件机制通知组件系统应用快照
 * 设计原则：避免循环依赖，保持架构简洁
 */
template<typename DataT> class ComponentSnapshot : public ISnapshot {
 public:
  /**
   * @brief 构造函数
   * @param entityId 关联的实体ID
   * @param data 组件数据副本（创建组件数据的深拷贝）
   */
  ComponentSnapshot(Entity entityId, const DataT &data)
      : m_entityId(entityId), m_snapshotData(data){}
  virtual ~ComponentSnapshot() = default;

  // ==================== ISnapshot接口实现 ====================
  /**
   * @brief 应用快照（重做操作）
   *
   * 通过事件总线发布快照应用事件，由对应的组件系统处理实际的应用逻辑
   * 这种设计避免了直接依赖，符合ECS架构原则
   */
  void Apply() override
  {
    // 发布快照应用事件，让组件系统来处理
    EventBus::Publish<ApplySnapshotEvent<DataT>>(
        ApplySnapshotEvent<DataT>(m_entityId, m_snapshotData));
  }
  /**
   * @brief 撤销快照（撤销操作）
   *
   * 对于组件快照，撤销和应用是相同的操作，都是将数据恢复到快照状态
   */
  void Revert() override
  {
    Apply();  // 撤销操作与应用操作相同
  }
  /**
   * @brief 获取快照内存使用量
   * @return size_t 快照数据占用的内存大小（字节）
   */
  size_t GetMemoryUsage() const override
  {
    return sizeof(DataT);
  }
  /**
   * @brief 获取快照描述信息
   * @return const char* 组件类型名称，用于调试和日志
   */
  const char *GetDescription() const override
  {
    return typeid(DataT).name();
  }
  // ==================== 数据访问接口 ====================
  /**
   * @brief 获取关联的实体ID
   * @return EntityID 快照对应的实体标识符
   */
  Entity GetEntityId() const
  {
    return m_entityId;
  }
  /**
   * @brief 获取快照数据
   * @return const DataT& 组件数据的常量引用
   */
  const DataT &GetData() const
  {
    return m_snapshotData;
  }

 private:
  Entity m_entityId;   ///< 关联的实体ID
  DataT m_snapshotData;  ///< 组件数据副本
};
/**
 * @brief 组件快照创建工具函数
 *
 * 提供类型安全的组件快照创建接口，简化使用方式
 *
 * @tparam DataT 组件数据类型
 * @param entityId 实体ID
 * @param data 组件数据
 * @return std::unique_ptr<ISnapshot> 创建的快照智能指针
 */
template<typename DataT>
std::unique_ptr<ComponentSnapshot<DataT>> CreateComponentSnapshot(Entity entityId,
                                                                       const DataT &data)
{
  return std::make_unique<ComponentSnapshot<DataT>>(entityId, data);
}

}  // namespace mite::scene

#endif  // MITE_SCENE_CORE_COMPONENT_SNAPSHOT
