#include "visibility_component.h"

namespace mite {
// ==================== 可见性操作 ====================
bool VisibilityComponent::IsVisible() const
{
  return m_Visibility.m_IsVisible;
}
void VisibilityComponent::SetVisible(bool visible)
{
  if (m_Visibility.m_IsVisible != visible) {
    m_Visibility.m_IsVisible = visible;
    EventBus::Publish<VisibilityChangedEvent>(GetEntity(), *this);
  }
}
// ==================== 掩码操作 ====================
uint32_t VisibilityComponent::GetVisibilityMask() const
{
  return m_Visibility.m_VisibilityMask;
}
void VisibilityComponent::SetVisibilityMask(uint32_t mask)
{
  if (m_Visibility.m_VisibilityMask != mask) {
    m_Visibility.m_VisibilityMask = mask;

    // 发布掩码改变事件
    EventBus::Publish<VisibilityChangedEvent>(GetEntity(), *this);
  }
}
bool VisibilityComponent::MatchesMask(uint32_t cameraMask) const
{
  return (m_Visibility.m_VisibilityMask & cameraMask) != 0;
}
void VisibilityComponent::AddMaskBits(uint32_t maskBits)
{
  SetVisibilityMask(m_Visibility.m_VisibilityMask | maskBits);
}
void VisibilityComponent::RemoveMaskBits(uint32_t maskBits)
{
  SetVisibilityMask(m_Visibility.m_VisibilityMask & ~maskBits);
}

// ==================== 组件接口 ====================
std::vector<std::type_index> VisibilityComponent::GetDependencies() const
{
  return {};
}
const Visibility &VisibilityComponent::GetSnapshotData() const
{
  return m_Visibility;
}

void VisibilityComponent::SetSnapshotData(const Visibility &data)
{
  m_Visibility = data;
  // 发布更新事件
  EventBus::Publish<VisibilityChangedEvent>(GetEntity(), *this);
}

bool VisibilityComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);
  // 序列化基础数据
  // TODO: 实现具体的序列化逻辑
  return !output.fail();
}

bool VisibilityComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);
  // 反序列化基础数据
  // TODO: 实现具体的反序列化逻辑
  return !input.fail();
}
}  // namespace mite