#include "ui_viewport_input_context.h"

namespace mite {
ViewportInputContext::ViewportInputContext(const std::string &name) : InputContext(name) {}

void ViewportInputContext::SetViewportFocus(bool focused)
{
  m_ViewportFocused = focused;
  if (!focused) {
    // 失去焦点时重置交互状态
    m_IsDragging = false;
  }
}

void ViewportInputContext::SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size)
{
  m_ViewportPos = pos;
  m_ViewportSize = size;
}

bool ViewportInputContext::IsMouseInViewport(const glm::vec2 &mousePos) const
{
  return mousePos.x >= m_ViewportPos.x && mousePos.x <= m_ViewportPos.x + m_ViewportSize.x &&
         mousePos.y >= m_ViewportPos.y && mousePos.y <= m_ViewportPos.y + m_ViewportSize.y;
}

glm::vec2 ViewportInputContext::ScreenToViewport(const glm::vec2 &screenPos) const
{
  return glm::vec2(screenPos.x - m_ViewportPos.x, screenPos.y - m_ViewportPos.y);
}

void ViewportInputContext::ProcessMouseMoveEvent(MouseMoveEvent &e)
{
  if (!m_ViewportFocused)
    return;

  glm::vec2 mousePos = e.GetPosition();
  if (!IsMouseInViewport(mousePos))
    return;

}

void ViewportInputContext::ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e)
{
  if (!m_ViewportFocused)
    return;

  glm::vec2 mousePos = {e.GetXPos(), e.GetYPos()};
  if (!IsMouseInViewport(mousePos))
    return;

}

void ViewportInputContext::ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e)
{
}

void ViewportInputContext::ProcessMouseScrollEvent(MouseScrollEvent &e)
{
  if (!m_ViewportFocused)
    return;
}

void ViewportInputContext::ProcessKeyPressdEvent(KeyPressedEvent &e)
{
  if (!m_ViewportFocused)
    return;

}

void ViewportInputContext::ProcessKeyReleasedEvent(KeyReleasedEvent &e)
{
  // 视口键盘释放事件处理
}

void ViewportInputContext::ProcessKeyTypedEvent(KeyTypedEvent &e)
{
  // 字符输入处理（如重命名选中的节点）
}

void ViewportInputContext::HandleNodeSelection(const glm::vec2 &viewportPos)
{
  // TODO: 实现节点选择逻辑
  // 1. 将视口坐标转换为世界坐标射线
  // 2. 执行射线与场景物体的碰撞检测
  // 3. 选择最近的相交物体
  // 4. 发布节点选择事件

  LOG_DEBUG("Node selection at viewport position: ({}, {})", viewportPos.x, viewportPos.y);
}

void ViewportInputContext::HandleNodeDrag(const glm::vec2 &viewportPos, const glm::vec2 &delta)
{
  // TODO: 实现节点拖动逻辑
  // 1. 根据拖动delta计算世界空间位移
  // 2. 更新选中节点的位置
  // 3. 发布节点变换事件

  LOG_DEBUG("Node dragging - delta: ({}, {})", delta.x, delta.y);
}
}  // namespace mite