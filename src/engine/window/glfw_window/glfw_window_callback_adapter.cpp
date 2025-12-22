#include "glfw_window_callback_adapter.h"
#include "input/input_event.h"
#include "window_event.h"
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

  // 注册鼠标/键盘事件回调（因为glfw无法生产imgui独立窗口的事件，该功能由UIImguiInputProducer接管）
  // glfwSetCursorPosCallback(window, &HandleMouseMove);
  // glfwSetMouseButtonCallback(window, &HandleMouseButton);
  // glfwSetScrollCallback(window, &HandleMouseScroll);
  // glfwSetKeyCallback(window, &HandleKeyEvent);
  // glfwSetCharCallback(window, &HandleCharInput);
}
void GLFWWindowCallbackAdapter::UnregisterCallbacks()
{
  if (!m_Window)
    return;

  // 重置窗口事件回调
  glfwSetWindowCloseCallback(m_Window, nullptr);
  glfwSetWindowSizeCallback(m_Window, nullptr);
  glfwSetWindowFocusCallback(m_Window, nullptr);
  glfwSetWindowPosCallback(m_Window, nullptr);

  // 重置鼠标/键盘事件回调（因为glfw无法生产imgui独立窗口的事件，该功能由UIImguiInputProducer接管）
  // glfwSetCursorPosCallback(m_Window, nullptr);
  // glfwSetMouseButtonCallback(m_Window, nullptr);
  // glfwSetScrollCallback(m_Window, nullptr);
  // glfwSetKeyCallback(m_Window, nullptr);
  // glfwSetCharCallback(m_Window, nullptr);

  m_Window = nullptr;
}
void GLFWWindowCallbackAdapter::ErrorCallback(int error, const char *description)
{
  s_Logger->error("GLFW Error ({}): {}", error, description);
}
void GLFWWindowCallbackAdapter::HandleWindowClose([[maybe_unused]] GLFWwindow *window)
{
  EventBus::Publish<WindowCloseEvent>();
  s_Logger->info("Window close requested");
}
void GLFWWindowCallbackAdapter::HandleWindowResize([[maybe_unused]] GLFWwindow *window,
                                                   int width,
                                                   int height)
{
  EventBus::Publish<WindowResizeEvent>(width, height);
  // 避免过多的窗口Resize日志，在调试时选择性启用
  // s_Logger->debug("Framebuffer resized to {}x{}", width, height);
}
void GLFWWindowCallbackAdapter::HandleWindowFocus([[maybe_unused]] GLFWwindow *window, int focused)
{
  if (focused == GLFW_TRUE) {
    EventBus::Publish<WindowFocusEvent>();
    s_Logger->debug("Window focus requested");
  }
  else {
    EventBus::Publish<WindowLostFocusEvent>();
    s_Logger->debug("Window focus losted");
  }
}
void GLFWWindowCallbackAdapter::HandleWindowMoved([[maybe_unused]] GLFWwindow *window,
                                                  int xpos,
                                                  int ypos)
{
  EventBus::Publish<WindowMovedEvent>(xpos, ypos);
  // 避免过多的窗口移动日志，在调试时选择性启用
  // s_Logger->debug("Window moved to {}, {}", xpos, ypos);
}
void GLFWWindowCallbackAdapter::HandleMouseMove([[maybe_unused]] GLFWwindow *window,
                                                double xpos,
                                                double ypos)
{
  EventBus::Publish<MouseMoveEvent>(static_cast<float>(xpos), static_cast<float>(ypos));
  // 避免过多的鼠标移动日志，在调试时选择性启用
  // s_Logger->debug("Mouse moved to ({}, {})", xpos, ypos);
}
void GLFWWindowCallbackAdapter::HandleMouseButton(GLFWwindow *window,
                                                  int button,
                                                  int action,
                                                  int mods)
{
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  if (action == GLFW_PRESS) {
    EventBus::Publish<MouseButtonPressedEvent>(
        button, mods, static_cast<float>(xpos), static_cast<float>(ypos));
    // 避免过多的Mouse Button日志，在调试时选择性启用
    // s_Logger->debug("Mouse button {} pressed", button);
  }
  else {
    EventBus::Publish<MouseButtonReleasedEvent>(
        button, static_cast<float>(xpos), static_cast<float>(ypos));
    // 避免过多的Mouse Button日志，在调试时选择性启用
    // s_Logger->debug("Mouse button {} released", button);
  }
}
void GLFWWindowCallbackAdapter::HandleMouseScroll([[maybe_unused]] GLFWwindow *window,
                                                  double xoffset,
                                                  double yoffset)
{
  EventBus::Publish<MouseScrollEvent>(xoffset, yoffset);
  // 避免过多的滚轮日志，在调试时选择性启用
  // s_Logger->debug("Mouse scrolled: xoffset={}, yoffset={}", xoffset, yoffset);
}
void GLFWWindowCallbackAdapter::HandleKeyEvent(
    [[maybe_unused]] GLFWwindow *window, int key, int scancode, int action, int mods)
{
  switch (action) {
    case GLFW_PRESS: {
      EventBus::Publish<KeyPressedEvent>(key, mods, false);  // 非重复按键
      s_Logger->trace("Key pressed: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
    case GLFW_RELEASE: {
      EventBus::Publish<KeyReleasedEvent>(key);
      // 避免过多的Key released日志，在调试时选择性启用
      s_Logger->trace("Key released: {}", key);
      break;
    }
    case GLFW_REPEAT: {
      EventBus::Publish<KeyPressedEvent>(key, mods, true);  // 重复按键
      s_Logger->trace("Key pressed repeatly: {} (scancode: {}, mods: {})", key, scancode, mods);
      break;
    }
  }
}
void GLFWWindowCallbackAdapter::HandleCharInput([[maybe_unused]] GLFWwindow *window,
                                                unsigned int codepoint)
{
  EventBus::Publish<KeyTypedEvent>(static_cast<char>(codepoint));
  s_Logger->debug("Key typed: {}", static_cast<char>(codepoint));
}
GLFWWindowCallbackAdapter *GLFWWindowCallbackAdapter::GetAdapter(GLFWwindow *window)
{
  auto *adapter = static_cast<GLFWWindowCallbackAdapter *>(glfwGetWindowUserPointer(window));
  assert(adapter && "GLFW window user pointer not set!");
  return adapter;
}
};  // namespace mite