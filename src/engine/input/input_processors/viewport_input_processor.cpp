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

bool ViewportInputProcessor::handleMouseMove(MouseMoveEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleMouseMove(e);
}

bool ViewportInputProcessor::handleMouseButton(MouseButtonReleasedEvent &e)
{
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }

  const bool pressed = (e.GetEventType() == EventType::MOUSE_BUTTON_PRESSED);

  // 使用配置的导航按钮替代硬编码的右键
  if (e.GetButton() == m_NavigationButton) {
    m_InputState.rotating = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;
  }

  // 保留中键平移功能
  if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    m_InputState.panning = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;
  }

  return false;
}

bool ViewportInputProcessor::handleMouseScroll(MouseScrollEvent &e)
{
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleMouseScroll(e);
}

bool ViewportInputProcessor::handleKeyEvent(KeyReleasedEvent &e)
{
  if (!m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleKeyEvent(e);
}

}  // namespace mite