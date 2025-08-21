#include "viewport_input_processor.h"
#include "GLFW/glfw3.h"

namespace mite {
// 日志系统
Logger ViewportInputProcessor::s_Logger = nullptr;

ViewportInputProcessor::ViewportInputProcessor(std::shared_ptr<Camera> camera,
                                               int navigationButton = GLFW_MOUSE_BUTTON_RIGHT)
    : CameraInputProcessor(std::move(camera)), m_NavigationButton(navigationButton)
{
  // 首次创建时初始化日志系统
  if (!s_Logger) {
    s_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Processor: Viewport");
    s_Logger->trace("Created Input Processor: Viewport");
  }

  // 注意：
  // 此处无需重新订阅事件，事件触发后将会自动调用派生类的版本
  // 例如handleMouseMove()被重写后，MouseMoveEvent触发时，
  // 会调用ViewportInputProcessor::handleMouseMove。
}

bool ViewportInputProcessor::handleMouseMove(MouseMoveEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;  // 未处理该事件
  }
  return CameraInputProcessor::handleMouseMove(e);  // 返回父类处理结果
}

bool ViewportInputProcessor::handleMouseButtonPressed(MouseButtonPressedEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }

  const bool pressed = true;

  // 使用配置的导航按钮替代硬编码的右键
  if (e.GetButton() == m_NavigationButton) {
    m_InputState.rotating = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;  // 已处理该事件
  }

  // 保留中键平移功能
  if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    m_InputState.panning = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;  // 已处理该事件
  }

  return false;  // 未处理其他按钮事件
}

bool ViewportInputProcessor::handleMouseButtonReleased(MouseButtonReleasedEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }

  const bool pressed = false;

  // 使用配置的导航按钮替代硬编码的右键
  if (e.GetButton() == m_NavigationButton) {
    m_InputState.rotating = pressed;
    return true;  // 已处理该事件
  }

  // 保留中键平移功能
  if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    m_InputState.panning = pressed;
    return true;  // 已处理该事件
  }

  return false;  // 未处理其他按钮事件
}

bool ViewportInputProcessor::handleMouseScroll(MouseScrollEvent &e)
{
  // 只在视口有焦点且悬停时处理
  if (!m_ViewportHovered || !m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleMouseScroll(e);  // 返回父类处理结果
}

bool ViewportInputProcessor::handleKeyPressedEvent(KeyPressedEvent &e)
{
  // 只在视口有焦点时处理
  if (!m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleKeyPressedEvent(e);  // 返回父类处理结果
}

bool ViewportInputProcessor::handleKeyReleasedEvent(KeyReleasedEvent &e)
{
  // 只在视口有焦点时处理
  if (!m_ViewportFocused) {
    return false;
  }
  return CameraInputProcessor::handleKeyReleasedEvent(e);  // 返回父类处理结果
}

}  // namespace mite