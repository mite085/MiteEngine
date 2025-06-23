#include "glfw_window_callback_adapter.h"
#include "window_event.h"
#include "input_event.h"
namespace mite {
GLFWWindowCallbackAdapter::~GLFWWindowCallbackAdapter()
{
  UnregisterCallbacks();  // 确保注销所有回调
}
void GLFWWindowCallbackAdapter::RegisterCallbacks(GLFWwindow *window)
{
  m_Window = window;
  glfwSetWindowUserPointer(window, this);

  // 注册窗口事件回调
  glfwSetWindowCloseCallback(window, &HandleWindowClose);
  glfwSetWindowSizeCallback(window, &HandleWindowResize);
  glfwSetWindowFocusCallback(window, &HandleWindowFocus);
  glfwSetWindowPosCallback(window, &HandleWindowMoved);

  // 注册鼠标事件回调
  glfwSetCursorPosCallback(window, &HandleMouseMove);
  glfwSetMouseButtonCallback(window, &HandleMouseButton);
  glfwSetScrollCallback(window, &HandleMouseScroll);

  // 注册键盘事件回调
  glfwSetKeyCallback(window, &HandleKeyEvent);
  glfwSetCharCallback(window, &HandleCharInput);
}
void GLFWWindowCallbackAdapter::UnregisterCallbacks()
{
  if (!m_Window)
    return;

  // 重置所有回调
  glfwSetWindowCloseCallback(m_Window, nullptr);
  glfwSetWindowSizeCallback(m_Window, nullptr);
  glfwSetWindowFocusCallback(m_Window, nullptr);
  glfwSetWindowPosCallback(m_Window, nullptr);
  glfwSetCursorPosCallback(m_Window, nullptr);
  glfwSetMouseButtonCallback(m_Window, nullptr);
  glfwSetScrollCallback(m_Window, nullptr);
  glfwSetKeyCallback(m_Window, nullptr);
  glfwSetCharCallback(m_Window, nullptr);

  m_Window = nullptr;
}
void GLFWWindowCallbackAdapter::ErrorCallback(int error, const char *description)
{
  LOG_ERROR("GLFW Error ({}): {}", error, description);
}
void GLFWWindowCallbackAdapter::HandleWindowClose(GLFWwindow *window)
{
  auto *adapter = GetAdapter(window);
  WindowCloseEvent event;
  EventBus::Get().Post(event);
  LOG_INFO("Window close requested");
}
void GLFWWindowCallbackAdapter::HandleWindowResize(GLFWwindow *window, int width, int height)
{
  auto *adapter = GetAdapter(window);
  WindowResizeEvent event(width, height);
  EventBus::Get().Post(event);
  LOG_TRACE("Framebuffer resized to {}x{}", width, height);
}
void GLFWWindowCallbackAdapter::HandleWindowFocus(GLFWwindow *window, int focused)
{
  auto *adapter = GetAdapter(window);
  if (focused == GLFW_TRUE) {
    WindowFocusEvent event;
    EventBus::Get().Post(event);
    LOG_TRACE("Window focus requested");
  }
  else {
    WindowLostFocusEvent event;
    EventBus::Get().Post(event);
    LOG_TRACE("Window focus losted");
  }
}
void GLFWWindowCallbackAdapter::HandleWindowMoved(GLFWwindow *window, int xpos, int ypos)
{
  auto *adapter = GetAdapter(window);
  WindowMovedEvent event(xpos, ypos);
  EventBus::Get().Post(event);
  LOG_TRACE("Window moved to {}, {}", xpos, ypos);
}
void GLFWWindowCallbackAdapter::HandleMouseMove(GLFWwindow *window, double xpos, double ypos)
{
  auto *adapter = GetAdapter(window);
  MouseMoveEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
  EventBus::Get().Post(event);
  // 避免过多的鼠标移动日志，在调试时启用
  // LOG_TRACE("Mouse moved to ({}, {})", xpos, ypos);
}
void GLFWWindowCallbackAdapter::HandleMouseButton(GLFWwindow *window,
                                                  int button,
                                                  int action,
                                                  int mods)
{
  auto *adapter = GetAdapter(window);
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  if (action == GLFW_PRESS) {
    MouseButtonPressedEvent event(
        button, mods, static_cast<float>(xpos), static_cast<float>(ypos));
    EventBus::Get().Post(event);
    LOG_TRACE("Mouse button {} pressed", button);
  }
  else {
    MouseButtonReleasedEvent event(button, static_cast<float>(xpos), static_cast<float>(ypos));
    EventBus::Get().Post(event);
    LOG_TRACE("Mouse button {} released", button);
  }
}
void GLFWWindowCallbackAdapter::HandleMouseScroll(GLFWwindow *window,
                                                  double xoffset,
                                                  double yoffset)
{
  // TODO: 后续考虑实现细节
}
void GLFWWindowCallbackAdapter::HandleKeyEvent(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
  auto *adapter = GetAdapter(window);

  switch (action) {
    case GLFW_PRESS: {
      KeyPressedEvent event(key, mods, false);  // 非重复按键
      EventBus::Get().Post(event);
      LOG_TRACE("Key pressed: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
    case GLFW_RELEASE: {
      KeyReleasedEvent event(key);
      EventBus::Get().Post(event);
      LOG_TRACE("Key released: {}", key);
      break;
    }
    case GLFW_REPEAT: {
      KeyPressedEvent event(key, mods, true);  // 重复按键
      EventBus::Get().Post(event);
      LOG_TRACE("Key pressed repeatly: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
  }
}
void GLFWWindowCallbackAdapter::HandleCharInput(GLFWwindow *window, unsigned int codepoint)
{
  auto *adapter = GetAdapter(window);
  KeyTypedEvent event(static_cast<char>(codepoint));
  EventBus::Get().Post(event);
  LOG_TRACE("Key typed: {}", static_cast<char>(codepoint));
}
GLFWWindowCallbackAdapter *GLFWWindowCallbackAdapter::GetAdapter(GLFWwindow *window)
{
  auto *adapter = static_cast<GLFWWindowCallbackAdapter *>(glfwGetWindowUserPointer(window));
  assert(adapter && "GLFW window user pointer not set!");
  return adapter;
}
};  // namespace mite
