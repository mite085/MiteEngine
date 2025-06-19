#include "visibility_component.h"

namespace mite {
// 静态断言确保枚举大小
static_assert(sizeof(VisibilityComponent::State) == sizeof(uint8_t),
              "VisibilityComponent::State size mismatch");

VisibilityComponent::VisibilityComponent()
    : ComponentTraits(),
      m_VisibilityState(State::FullyVisible),
      m_TargetVisibilityState(State::FullyVisible),
      m_CurrentOpacity(1.0f),
      m_TargetOpacity(1.0f),
      m_VisibilityTransitionTime(0.0f),
      m_OpacityTransitionTime(0.0f),
      m_OpacityTransitionSpeed(0.0f)
{
  // 确保初始状态一致
  assert(m_CurrentOpacity >= 0.0f && m_CurrentOpacity <= 1.0f);
}

VisibilityComponent::VisibilityComponent(State initialVisibility, float initialOpacity)
    : ComponentTraits(),
      m_VisibilityState(initialVisibility),
      m_TargetVisibilityState(initialVisibility),
      m_CurrentOpacity(initialOpacity),
      m_TargetOpacity(initialOpacity),
      m_VisibilityTransitionTime(0.0f),
      m_OpacityTransitionTime(0.0f),
      m_OpacityTransitionSpeed(0.0f)
{
  assert(initialOpacity >= 0.0f && initialOpacity <= 1.0f);
}

void VisibilityComponent::SetVisibilityState(State state, float transitionTime)
{
  if (m_VisibilityState == state && m_VisibilityTransitionTime <= 0.0f) {
    return;  // 状态未改变且没有进行中的过渡
  }

  m_TargetVisibilityState = state;
  m_VisibilityTransitionTime = transitionTime;

  // 立即切换条件
  if (transitionTime <= 0.0f) {
    m_VisibilityState = state;
    m_VisibilityTransitionTime = 0.0f;
  }
}

void VisibilityComponent::SetOpacity(float opacity, float transitionTime)
{
  assert(opacity >= 0.0f && opacity <= 1.0f);

  if (std::abs(m_TargetOpacity - opacity) < FLT_EPSILON && m_OpacityTransitionTime <= 0.0f) {
    return;  // 目标值未改变且没有进行中的过渡
  }

  m_TargetOpacity = opacity;
  m_OpacityTransitionTime = transitionTime;

  // 计算过渡速度
  if (transitionTime > 0.0f) {
    m_OpacityTransitionSpeed = std::abs(m_TargetOpacity - m_CurrentOpacity) / transitionTime;
  }
  else {
    m_CurrentOpacity = opacity;
    m_OpacityTransitionSpeed = 0.0f;
  }
}

bool VisibilityComponent::IsVisible() const
{
  return m_VisibilityState != State::Hidden && m_VisibilityState != State::Culled;
}

bool VisibilityComponent::IsFullyVisible() const
{
  return m_VisibilityState == State::FullyVisible &&
         std::abs(m_CurrentOpacity - 1.0f) < FLT_EPSILON;
}

void VisibilityComponent::Update(float deltaTime)
{
  // 更新可见性状态过渡
  if (m_VisibilityTransitionTime > 0.0f) {
    m_VisibilityTransitionTime -= deltaTime;
    if (m_VisibilityTransitionTime <= 0.0f) {
      m_VisibilityState = m_TargetVisibilityState;
      m_VisibilityTransitionTime = 0.0f;
    }
  }

  // 更新透明度过渡
  if (m_OpacityTransitionTime > 0.0f) {
    const float deltaOpacity = m_OpacityTransitionSpeed * deltaTime;

    if (m_TargetOpacity > m_CurrentOpacity) {
      m_CurrentOpacity = std::min(m_CurrentOpacity + deltaOpacity, m_TargetOpacity);
    }
    else {
      m_CurrentOpacity = std::max(m_CurrentOpacity - deltaOpacity, m_TargetOpacity);
    }

    m_OpacityTransitionTime -= deltaTime;
    if (m_OpacityTransitionTime <= 0.0f) {
      m_CurrentOpacity = m_TargetOpacity;
      m_OpacityTransitionTime = 0.0f;
      m_OpacityTransitionSpeed = 0.0f;
    }
  }
}

void VisibilityComponent::CompleteTransitions()
{
  m_VisibilityState = m_TargetVisibilityState;
  m_VisibilityTransitionTime = 0.0f;

  m_CurrentOpacity = m_TargetOpacity;
  m_OpacityTransitionTime = 0.0f;
  m_OpacityTransitionSpeed = 0.0f;
}
};  // namespace mite