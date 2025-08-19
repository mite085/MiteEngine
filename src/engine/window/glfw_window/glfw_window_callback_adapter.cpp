#include "glfw_window_callback_adapter.h"
#include "window_event.h"
#include "input/input_event.h"
namespace mite {

Logger GLFWWindowCallbackAdapter::s_Logger = nullptr;

GLFWWindowCallbackAdapter::GLFWWindowCallbackAdapter() : CallbackAdapter()
{  
  // 首次创建时初始化日志系统
  if (!s_Logger) {
    s_Logger = mite::LoggerSystem::CreateModuleLogger("Mite GLFW Window Callback Adapter");
    s_Logger->trace("Created GLFW Window Callback Adapter");
  }
}

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
  s_Logger->error("GLFW Error ({}): {}", error, description);
}
void GLFWWindowCallbackAdapter::HandleWindowClose(GLFWwindow *window)
{
  auto *adapter = GetAdapter(window);
  WindowCloseEvent event;
  EventBus::Get().Post(event);
  s_Logger->info("Window close requested");
}
void GLFWWindowCallbackAdapter::HandleWindowResize(GLFWwindow *window, int width, int height)
{
  auto *adapter = GetAdapter(window);
  WindowResizeEvent event(width, height);
  EventBus::Get().Post(event);
  // 避免过多的窗口Resize日志，在调试时选择性启用
  // s_Logger->debug("Framebuffer resized to {}x{}", width, height);
}
void GLFWWindowCallbackAdapter::HandleWindowFocus(GLFWwindow *window, int focused)
{
  auto *adapter = GetAdapter(window);
  if (focused == GLFW_TRUE) {
    WindowFocusEvent event;
    EventBus::Get().Post(event);
    s_Logger->debug("Window focus requested");
  }
  else {
    WindowLostFocusEvent event;
    EventBus::Get().Post(event);
    s_Logger->debug("Window focus losted");
  }
}
void GLFWWindowCallbackAdapter::HandleWindowMoved(GLFWwindow *window, int xpos, int ypos)
{
  auto *adapter = GetAdapter(window);
  WindowMovedEvent event(xpos, ypos);
  EventBus::Get().Post(event);
  // 避免过多的窗口移动日志，在调试时选择性启用
  //s_Logger->debug("Window moved to {}, {}", xpos, ypos);
}
void GLFWWindowCallbackAdapter::HandleMouseMove(GLFWwindow *window, double xpos, double ypos)
{
  auto *adapter = GetAdapter(window);
  MouseMoveEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
  EventBus::Get().Post(event);
  // 避免过多的鼠标移动日志，在调试时选择性启用
  // s_Logger->debug("Mouse moved to ({}, {})", xpos, ypos);
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
    // 避免过多的Mouse Button日志，在调试时选择性启用
    // s_Logger->debug("Mouse button {} pressed", button);
  }
  else {
    MouseButtonReleasedEvent event(button, static_cast<float>(xpos), static_cast<float>(ypos));
    EventBus::Get().Post(event);
    // 避免过多的Mouse Button日志，在调试时选择性启用
    // s_Logger->debug("Mouse button {} released", button);
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
      // 避免过多的Key pressed日志，在调试时选择性启用
      // s_Logger->debug("Key pressed: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
    case GLFW_RELEASE: {
      KeyReleasedEvent event(key);
      EventBus::Get().Post(event);
      // 避免过多的Key released日志，在调试时选择性启用
      // s_Logger->debug("Key released: {}", key);
      break;
    }
    case GLFW_REPEAT: {
      KeyPressedEvent event(key, mods, true);  // 重复按键
      EventBus::Get().Post(event);
      // 避免过多的Key pressed日志，在调试时选择性启用
      // s_Logger->debug("Key pressed repeatly: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
  }
}
void GLFWWindowCallbackAdapter::HandleCharInput(GLFWwindow *window, unsigned int codepoint)
{
  auto *adapter = GetAdapter(window);
  KeyTypedEvent event(static_cast<char>(codepoint));
  EventBus::Get().Post(event);
  s_Logger->debug("Key typed: {}", static_cast<char>(codepoint));
}
GLFWWindowCallbackAdapter *GLFWWindowCallbackAdapter::GetAdapter(GLFWwindow *window)
{
  auto *adapter = static_cast<GLFWWindowCallbackAdapter *>(glfwGetWindowUserPointer(window));
  assert(adapter && "GLFW window user pointer not set!");
  return adapter;
}
};  // namespace mite
