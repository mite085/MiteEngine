#include "ui_widget.h"
#include "ui_event/ui_events_layout.h"

namespace mite {

UIWidget::UIWidget(const std::string &name)
    : m_ID(UUIDGenerator::Generate()),
      m_Name(name),
      m_Position(0.0f, 0.0f),
      m_Size(100.0f, 30.0f),
      m_Visible(true),
      m_Enabled(true),
      m_Style(nullptr)
{
}

UIWidget::~UIWidget()
{
  // 清理资源
}

void UIWidget::SetPosition(const glm::vec2 &position)
{
  if (m_Position != position) {
    glm::vec2 oldPosition = m_Position;
    m_Position = position;
    EventBus::Publish<WidgetPositionChangedEvent>(
        WidgetPositionChangedEvent(m_ID, oldPosition, position));
  }
}
void UIWidget::SetSize(const glm::vec2 &size)
{
  if (m_Size != size) {
    glm::vec2 oldSize = m_Size;
    m_Size = size;

    EventBus::Publish<WidgetSizeChangedEvent>(WidgetSizeChangedEvent(m_ID, oldSize, size));
  }
}
void UIWidget::SetVisible(bool visible)
{
  if (m_Visible != visible) {
    bool oldVisible = m_Visible;
    m_Visible = visible;

    EventBus::Publish<WidgetVisibilityChangedEvent>(
        WidgetVisibilityChangedEvent(m_ID, oldVisible, visible));
  }
}
void UIWidget::SetEnabled(bool enabled)
{
  if (m_Enabled != enabled) {
    bool oldEnabled = m_Enabled;
    m_Enabled = enabled;

    EventBus::Publish<WidgetEnabledStateChangedEvent>(
        WidgetEnabledStateChangedEvent(m_ID, oldEnabled, enabled));
  }
}
void UIWidget::SetStyle(std::shared_ptr<UIStyle> style)
{
  m_Style = style;
  // 触发布局更新请求，因为样式改变可能影响控件尺寸
  EventBus::Publish<LayoutUpdateRequestEvent>(LayoutUpdateRequestEvent(m_ID));
}

bool UIWidget::ContainsPoint(const glm::vec2 &point) const
{
  return point.x >= m_Position.x && point.x <= m_Position.x + m_Size.x &&
         point.y >= m_Position.y && point.y <= m_Position.y + m_Size.y;
}

void UIWidget::OnMouseEnter(const MouseEnterEvent &event)
{
  // 基类实现为空，子类可以重写
  LOG_DEBUG("Mouse entered widget: {}", m_Name);
}

void UIWidget::OnMouseLeave(const MouseLeaveEvent &event)
{
  // 基类实现为空，子类可以重写
  LOG_DEBUG("Mouse left widget: {}", m_Name);
}

void UIWidget::Update(float deltaTime)
{
  // 基类实现为空，子类可以重写处理动画或其他状态更新
}

}  // namespace mite
