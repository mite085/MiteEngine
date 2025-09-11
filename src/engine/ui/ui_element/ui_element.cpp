#include "ui_element.h"

namespace mite {

UIElement::UIElement(const std::string &name)
    : m_ID(UUIDGenerator::Generate()),
      m_Name(name),
      m_Position(0.0f, 0.0f),
      m_Size(0.0f, 0.0f),
      m_Visible(true),
      m_Enabled(true),
      m_Focused(false),
      m_Hovered(false)
{
  LOG_DEBUG("Created UIElement: {} (ID: {})", m_Name, UUIDGenerator::UUIDToString(m_ID));
}

UIElement::~UIElement()
{
  LOG_DEBUG("Destroyed UIElement: {} (ID: {})", m_Name, UUIDGenerator::UUIDToString(m_ID));
}

UUID UIElement::GetID() const
{
  return m_ID;
}

const std::string &UIElement::GetName() const
{
  return m_Name;
}

void UIElement::SetName(const std::string &name)
{
  if (m_Name != name) {
    std::string oldName = m_Name;
    m_Name = name;
    LOG_DEBUG("Renamed UIElement from '{}' to '{}'", oldName, m_Name);
  }
}

glm::vec2 UIElement::GetPosition() const
{
  return m_Position;
}

void UIElement::SetPosition(const glm::vec2 &position)
{
  if (m_Position != position) {
    glm::vec2 oldPosition = m_Position;
    m_Position = position;

    // 发布位置变化事件
    EventBus::Publish<ElementPositionChangedEvent>(
        ElementPositionChangedEvent(m_ID, oldPosition, position));
  }
}

glm::vec2 UIElement::GetSize() const
{
  return m_Size;
}

void UIElement::SetSize(const glm::vec2 &size)
{
  if (m_Size != size) {
    glm::vec2 oldSize = m_Size;
    m_Size = size;

    // 发布尺寸变化事件
    EventBus::Publish<ElementSizeChangedEvent>(ElementSizeChangedEvent(m_ID, oldSize, size));
  }
}

bool UIElement::IsVisible() const
{
  return m_Visible;
}

void UIElement::SetVisible(bool visible)
{
  if (m_Visible != visible) {
    bool oldVisible = m_Visible;
    m_Visible = visible;

    // 发布可见性变化事件
    EventBus::Publish<ElementVisibilityChangedEvent>(
        ElementVisibilityChangedEvent(m_ID, oldVisible, visible));
  }
}

bool UIElement::IsEnabled() const
{
  return m_Enabled;
}

void UIElement::SetEnabled(bool enabled)
{
  if (m_Enabled != enabled) {
    bool oldEnabled = m_Enabled;
    m_Enabled = enabled;

    // 发布启用状态变化事件
    EventBus::Publish<ElementEnabledStateChangedEvent>(
        ElementEnabledStateChangedEvent(m_ID, oldEnabled, enabled));
  }
}

bool UIElement::ContainsPoint(const glm::vec2 &point) const
{
  return point.x >= m_Position.x && point.x <= m_Position.x + m_Size.x &&
         point.y >= m_Position.y && point.y <= m_Position.y + m_Size.y;
}

void UIElement::OnMouseEnter(const MouseEnterEvent &event)
{
  SetHovered(true);
  LOG_DEBUG("Mouse entered element: {}", m_Name);
}

void UIElement::OnMouseLeave(const MouseLeaveEvent &event)
{
  SetHovered(false);
  LOG_DEBUG("Mouse left element: {}", m_Name);
}

glm::vec4 UIElement::GetBounds() const
{
  return glm::vec4(m_Position.x, m_Position.y, m_Size.x, m_Size.y);
}

void UIElement::SetFocused(bool focused)
{
  if (m_Focused != focused) {
    m_Focused = focused;

    // 发布焦点变化事件
    if (m_Focused)
      EventBus::Publish<UIFocusGainedEvent>(UIFocusGainedEvent(m_ID));
    else
      EventBus::Publish<UIFocusLostEvent>(UIFocusLostEvent(m_ID));
  }
}

bool UIElement::IsFocused() const
{
  return m_Focused;
}

void UIElement::SetHovered(bool hovered)
{
  if (m_Hovered != hovered) {
    m_Hovered = hovered;

    // 发布悬停状态变化事件
    EventBus::Publish<ElementHoverStateChangedEvent>(ElementHoverStateChangedEvent(m_ID, hovered));
  }
}

bool UIElement::IsHovered() const
{
  return m_Hovered;
}
}  // namespace mite
