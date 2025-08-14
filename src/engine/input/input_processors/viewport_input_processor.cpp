#include "viewport_input_processor.h"
#include "GLFW/glfw3.h"

namespace mite {

ViewportInputProcessor::ViewportInputProcessor(std::shared_ptr<Camera> camera,
                                               int navigationButton = GLFW_MOUSE_BUTTON_RIGHT)
    : CameraInputProcessor(std::move(camera)), m_NavigationButton(navigationButton)
{
    // 注意：
    // 此处无需重新订阅事件，事件触发后将会自动调用派生类的版本
    // 例如handleMouseMove()被重写后，MouseMoveEvent触发时，
    // 会调用ViewportInputProcessor::handleMouseMove。
}

void ViewportInputProcessor::handleMouseMove(MouseMoveEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return;
  }
  return CameraInputProcessor::handleMouseMove(e);
}

void ViewportInputProcessor::handleMouseButton(MouseButtonReleasedEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return;
  }

  const bool pressed = (e.GetEventType() == EventType::MOUSE_BUTTON_PRESSED);

  // 使用配置的导航按钮替代硬编码的右键
  if (e.GetButton() == m_NavigationButton) {
    m_InputState.rotating = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return;
  }

  // 保留中键平移功能
  if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    m_InputState.panning = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return;
  }
}

void ViewportInputProcessor::handleMouseScroll(MouseScrollEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return;
  }
  return CameraInputProcessor::handleMouseScroll(e);
}

void ViewportInputProcessor::handleKeyEvent(KeyReleasedEvent &e)
{
  // 只在视口有焦点时处理
  if (!m_ViewportFocused) {
    return;
  }
  return CameraInputProcessor::handleKeyEvent(e);
}

}  // namespace mite