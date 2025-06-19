#ifndef MITE_VISIBILITY_COMPONENT
#define MITE_VISIBILITY_COMPONENT

#include "scene_core/component.h"

namespace mite {
/**
 * @brief 可见性组件，控制实体在场景中的可见性状态
 *
 * 提供多层次的可见性控制，支持运行时动态修改和动画混合。
 * 继承自ComponentTraits，使用Core组件家族。
 */
class VisibilityComponent : public ComponentTraits<VisibilityComponent, Component::Family::Core> {
 public:
  // 可见性状态枚举
  enum class State : uint8_t {
    FullyVisible,      // 完全可见
    PartiallyVisible,  // 部分可见(用于过渡效果)
    Hidden,            // 完全隐藏但仍参与计算
    Culled             // 被视锥剔除且不参与计算
  };

  /**
   * @brief 默认构造函数
   *
   * 注意：默认状态为FullyVisible，透明度为1.0
   */
  VisibilityComponent();

  /**
   * @brief 带参数的构造函数
   * @param initialVisibility 初始可见性状态
   * @param initialOpacity 初始透明度(0.0-1.0)
   */
  explicit VisibilityComponent(State initialVisibility, float initialOpacity = 1.0f);

  ~VisibilityComponent() = default;

  // 禁止拷贝和移动(由ECS管理)
  VisibilityComponent(const VisibilityComponent &) = delete;
  VisibilityComponent &operator=(const VisibilityComponent &) = delete;
  VisibilityComponent(VisibilityComponent &&) = delete;
  VisibilityComponent &operator=(VisibilityComponent &&) = delete;

  /**
   * @brief 获取当前可见性状态
   * @return 可见性状态枚举值
   */
  State GetVisibilityState() const
  {
    return m_VisibilityState;
  }

  /**
   * @brief 设置可见性状态
   * @param state 新的可见性状态
   * @param transitionTime 过渡时间(秒)，0表示立即切换
   */
  void SetVisibilityState(State state, float transitionTime = 0.0f);

  /**
   * @brief 获取当前透明度
   * @return 透明度值(0.0-1.0)
   */
  float GetOpacity() const
  {
    return m_CurrentOpacity;
  }

  /**
   * @brief 设置目标透明度
   * @param opacity 目标透明度(0.0-1.0)
   * @param transitionTime 过渡时间(秒)，0表示立即切换
   */
  void SetOpacity(float opacity, float transitionTime = 0.0f);

  /**
   * @brief 检查是否可见(包含部分可见状态)
   * @return 如果状态不是Hidden或Culled返回true
   */
  bool IsVisible() const;

  /**
   * @brief 检查是否完全可见
   * @return 状态为FullyVisible且透明度为1.0返回true
   */
  bool IsFullyVisible() const;

  /**
   * @brief 更新组件状态(每帧调用)
   * @param deltaTime 帧时间(秒)
   */
  void Update(float deltaTime);

  /**
   * @brief 强制立即完成所有过渡动画
   */
  void CompleteTransitions();

 private:
  State m_VisibilityState;        // 当前可见性状态
  State m_TargetVisibilityState;  // 目标可见性状态(用于过渡)

  float m_CurrentOpacity;  // 当前透明度值(0.0-1.0)
  float m_TargetOpacity;   // 目标透明度值(0.0-1.0)

  float m_VisibilityTransitionTime;  // 可见性状态过渡剩余时间(秒)
  float m_OpacityTransitionTime;     // 透明度过渡剩余时间(秒)

  float m_OpacityTransitionSpeed;  // 透明度过渡速度(每秒变化量)
};
};  // namespace mite

#endif
