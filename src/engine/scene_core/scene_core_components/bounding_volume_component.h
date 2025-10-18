#ifndef MITE_BOUNDING_VOLUME_COMPONENT_H
#define MITE_BOUNDING_VOLUME_COMPONENT_H

#include "basic_data/bounding_volume.h"
#include "scene_core/component_system.h"

namespace mite {

/**
 * @class BoundingVolumeComponent
 * @brief 包围体组件，仅存储局部空间的包围体信息
 *
 * 职责：
 * 1. 存储实体局部空间的包围体数据
 * 2. 提供简单的数据存取接口
 * 3. 管理组件的序列化和脏标记
 */
class BoundingVolumeComponent
    : public SnapshotComponentTraits<BoundingVolume, Component::Family::Geometry> {
 public:
  /**
   * @brief 默认构造函数
   */
  BoundingVolumeComponent();

  ~BoundingVolumeComponent() override = default;

  // ==================== 数据访问接口 ====================

  /**
   * @brief 获取局部空间包围体
   * @return 包围体引用
   */
  const BoundingVolume &GetVolume() const
  {
    return m_Volume;
  }

  /**
   * @brief 设置局部空间包围体
   * @param volume 新的包围体
   */
  void SetVolume(const BoundingVolume &volume);

  /**
   * @brief 获取包围体类型
   * @return 包围体类型
   */
  BoundingVolumeType GetVolumeType() const
  {
    return m_Volume.GetType();
  }

  // ==================== 组件接口 ====================

  /**
   * @brief 获取组件依赖
   * @return 依赖组件类型列表
   */
  std::vector<std::type_index> GetDependencies() const override;

  /**
   * @brief 序列化组件数据
   */
  bool Serialize(std::ostream &output) const override;

  /**
   * @brief 反序列化组件数据
   */
  bool Deserialize(std::istream &input) override;

 private:
  BoundingVolume GetSnapshotData() const override;
  void SetSnapshotData(const BoundingVolume &data) override;

  BoundingVolume m_Volume;  // 局部空间包围体
};

// ==================== 组件系统 ====================

/**
 * @class BoundingVolumeComponentSystem
 * @brief 包围体组件系统，负责管理包围体状态
 */
class BoundingVolumeComponentSystem : public SnapshotComponentSystem<BoundingVolumeComponent> {
  DECLARE_COMPONENT_SYSTEM(BoundingVolumeComponentSystem)
};

// ==================== 事件定义 ====================

/**
 * @class BoundingVolumeChangedEvent
 * @brief 包围体改变事件
 */
class BoundingVolumeChangedEvent : public ComponentEvent<BoundingVolumeComponent> {
 public:
  BoundingVolumeChangedEvent(Entity entity,
                             BoundingVolumeComponent &component)
      : ComponentEvent<BoundingVolumeComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

  Event *Clone() const override
  {
    return new BoundingVolumeChangedEvent(entity, component);
  }
};

}  // namespace mite

#endif  // MITE_BOUNDING_VOLUME_COMPONENT_H
