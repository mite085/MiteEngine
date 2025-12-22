#ifndef MITE_VISIBILITY_COMPONENT_H
#define MITE_VISIBILITY_COMPONENT_H

#include "basic_type/visible_type.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @class VisibilityComponent
 * @brief 可见性组件，专注于管理实体的可见性掩码和可见性状态
 *
 * 功能特性：
 * 1. 提供可见性掩码支持分层渲染
 * 2. 维护“可见/不可见”的可见性状态
 * 3. 与SceneGraph协同实现高效的可见性管理
 *
 * 注意：相机实体也应当持有该组件，但相机不使用bool的可见性判断，仅使用可见性掩码进行匹配。
 */
class VisibilityComponent
    : public SnapshotComponentTraits<Visibility,
                                     Component::Family::Visibility> {
 public:
  /**
   * @brief 默认构造函数
   */
  VisibilityComponent() = default;

  ~VisibilityComponent() override = default;

  // ==================== 可见性操作 ====================
  /**
   * @brief 获取当前可见性状态
   * @return 是否可见
   */
  bool IsVisible() const;
  /**
   * @brief 设置可见性状态（掩码无关的手动覆盖）
   * @param visible 是否可见
   */
  void SetVisible(bool visible);

  // ==================== 掩码操作 ====================
  /**
   * @brief 获取可见性掩码
   * @return 32位掩码
   */
  uint32_t GetVisibilityMask() const;
  /**
   * @brief 设置可见性掩码
   * @param mask 新的掩码
   */
  void SetVisibilityMask(uint32_t mask);
  /**
   * @brief 检查是否与给定掩码匹配
   * @param cameraMask 相机掩码
   * @return 是否匹配
   */
  bool MatchesMask(uint32_t cameraMask) const;
  /**
   * @brief 添加掩码位
   * @param maskBits 要添加的掩码位
   */
  void AddMaskBits(uint32_t maskBits);
  /**
   * @brief 移除掩码位
   * @param maskBits 要移除的掩码位
   */
  void RemoveMaskBits(uint32_t maskBits);

  // ==================== 组件接口 ====================

  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  const Visibility &GetSnapshotData() const override;
  void SetSnapshotData(const Visibility &data) override;

  Visibility m_Visibility;
};

// ==================== 组件系统 ====================

/**
 * @class VisibilityComponentSystem
 * @brief 可见性组件系统，负责批量处理可见性计算和剔除
 */
class VisibilityComponentSystem
    : public SnapshotComponentSystem<VisibilityComponent> {
  DECLARE_COMPONENT_SYSTEM(VisibilityComponentSystem)
};

// ==================== 事件定义 ====================

/**
 * @class VisibilityChangedEvent
 * @brief 可见性改变事件
 */
class VisibilityChangedEvent : public ComponentEvent<VisibilityComponent> {
 public:
  VisibilityChangedEvent(Entity entity, VisibilityComponent &component)
      : ComponentEvent<VisibilityComponent>(entity, component) {}
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

  Event *Clone() const override {
    return new VisibilityChangedEvent(this->m_Entity, this->m_Component);
  }
};
}  // namespace mite

#endif  // MITE_VISIBILITY_COMPONENT_H
