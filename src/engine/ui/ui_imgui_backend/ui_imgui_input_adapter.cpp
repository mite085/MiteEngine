#include "ui_imgui_input_adapter.h"

namespace mite {
ImGuiInputAdapter::ImGuiInputAdapter(const std::string &name) : ModularInputContext(name)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Input Adapter");
  m_Logger->info("Creating ImGuiInputAdapter: {}", name);
}

ImGuiInputAdapter::~ImGuiInputAdapter()
{
  m_Logger->info("Destroying ImGuiInputAdapter");
}

void ImGuiInputAdapter::Initialize()
{
  m_Logger->info("Initializing ImGuiInputAdapter");

  // 初始化鼠标状态
  m_LastMousePos = glm::vec2(0.0f);
}

void ImGuiInputAdapter::Shutdown()
{
  m_Logger->info("Shutting down ImGuiInputAdapter");
}

bool ImGuiInputAdapter::ProcessEvent(Event &e)
{
  // 检查ImGui是否想要捕获该类型输入
  ImGuiIO &io = ImGui::GetIO();
  bool shouldCapture = false;

  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<MouseMoveEvent>([&](MouseMoveEvent &event) {
    shouldCapture = io.WantCaptureMouse;
    return ProcessMouseMoveEvent(event);
  });

  dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent &event) {
    shouldCapture = io.WantCaptureMouse;
    return ProcessMouseButtonEvent(event);
  });

  dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent &event) {
    shouldCapture = io.WantCaptureMouse;
    return ProcessMouseButtonEvent(event);
  });

  dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent &event) {
    shouldCapture = io.WantCaptureMouse;
    return ProcessMouseScrollEvent(event);
  });

  dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent &event) {
    shouldCapture = io.WantCaptureKeyboard;
    return ProcessKeyEvent(event);
  });

  dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent &event) {
    shouldCapture = io.WantCaptureKeyboard;
    return ProcessKeyEvent(event);
  });

  dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent &event) {
    shouldCapture = io.WantCaptureKeyboard;
    return ProcessKeyTypedEvent(event);
  });

  // 如果ImGui想要捕获该事件，标记为已处理
  if (shouldCapture && e.handled) {
    e.handled = true;
  }

  return e.handled;
}

void ImGuiInputAdapter::UpdateImGuiIO()
{
  // 1.87之后的ImGui，不需要每帧手动更新键盘状态
  // 所有键盘状态通过AddKeyEvent()实时添加
}

void ImGuiInputAdapter::UpdateDisplaySize(GLFWwindow *window)
{
  if (!window) {
    m_Logger->warn("GLFW window not available, cannot update display size");
    return;
  }
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  m_DisplaySize = glm::ivec2(width, height);
  // 更新ImGui的显示尺寸
  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(m_DisplaySize.x),
                          static_cast<float>(m_DisplaySize.y));
  m_Logger->debug("Display size updated: {}x{}", width, height);
}

void ImGuiInputAdapter::UpdateFramebufferScale(GLFWwindow *window)
{
  if (!window) {
    m_Logger->warn("GLFW window not available, cannot update framebuffer scale");
    return;
  }
  // 获取帧缓冲尺寸（实际渲染分辨率）
  int framebufferWidth, framebufferHeight;
  glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

  // 获取窗口尺寸（逻辑分辨率）
  int windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);
  // 计算缩放比例（处理高DPI显示）
  if (windowWidth > 0 && windowHeight > 0) {
    m_FramebufferScale = glm::vec2(
        static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth),
        static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight));
  }
  else {
    m_FramebufferScale = glm::vec2(1.0f);
  }
  // 更新ImGui的显示缩放
  ImGuiIO &io = ImGui::GetIO();
  io.DisplayFramebufferScale = ImVec2(m_FramebufferScale.x, m_FramebufferScale.y);
  m_Logger->debug(
      "Framebuffer scale updated: {:.2f}x{:.2f}", m_FramebufferScale.x, m_FramebufferScale.y);
}

// 具体事件处理实现
bool ImGuiInputAdapter::ProcessMouseMoveEvent(MouseMoveEvent &e)
{
  ImGuiIO &io = ImGui::GetIO();
  io.AddMousePosEvent(static_cast<float>(e.GetXPos()), static_cast<float>(e.GetYPos()));
  m_LastMousePos = glm::vec2(e.GetXPos(), e.GetYPos());
  return false;  // 不阻止事件继续传递
}

bool ImGuiInputAdapter::ProcessMouseButtonEvent(MouseButtonPressedEvent &e)
{
  ImGuiKey mouseKey = ConvertGlfwMouseButtonToImGuiKey(e.GetButton());
  if (mouseKey != ImGuiKey_None) {
    ImGui::GetIO().AddKeyEvent(mouseKey, true);
  }
  return false;
}

bool ImGuiInputAdapter::ProcessMouseButtonEvent(MouseButtonReleasedEvent &e)
{
  ImGuiKey mouseKey = ConvertGlfwMouseButtonToImGuiKey(e.GetButton());
  if (mouseKey != ImGuiKey_None) {
    ImGui::GetIO().AddKeyEvent(mouseKey, false);
  }
  return false;
}

bool ImGuiInputAdapter::ProcessMouseScrollEvent(MouseScrollEvent &e)
{
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseWheelEvent(static_cast<float>(e.GetXOffset()), static_cast<float>(e.GetYOffset()));
  return false;
}

bool ImGuiInputAdapter::ProcessKeyEvent(KeyPressedEvent &e)
{
  ImGuiKey key = ConvertGlfwKeyToImGuiKey(e.GetKey());
  if (key != ImGuiKey_None) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(key, true);

    // 处理修饰键状态
    io.AddKeyEvent(ImGuiKey_ModCtrl, (e.GetMods() & GLFW_MOD_CONTROL) != 0);
    io.AddKeyEvent(ImGuiKey_ModShift, (e.GetMods() & GLFW_MOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiKey_ModAlt, (e.GetMods() & GLFW_MOD_ALT) != 0);
    io.AddKeyEvent(ImGuiKey_ModSuper, (e.GetMods() & GLFW_MOD_SUPER) != 0);
  }
  return false;
}

bool ImGuiInputAdapter::ProcessKeyEvent(KeyReleasedEvent &e)
{
  ImGuiKey key = ConvertGlfwKeyToImGuiKey(e.GetKey());
  if (key != ImGuiKey_None) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(key, false);
  }
  return false;
}

bool ImGuiInputAdapter::ProcessKeyTypedEvent(KeyTypedEvent &e)
{
  ImGuiIO &io = ImGui::GetIO();
  io.AddInputCharacter(e.GetCodepoint());
  return false;
}

// GLFW键码到ImGuiKey的转换
ImGuiKey ImGuiInputAdapter::ConvertGlfwKeyToImGuiKey(int glfwKey)
{
  switch (glfwKey) {
    case GLFW_KEY_TAB:
      return ImGuiKey_Tab;
    case GLFW_KEY_LEFT:
      return ImGuiKey_LeftArrow;
    case GLFW_KEY_RIGHT:
      return ImGuiKey_RightArrow;
    case GLFW_KEY_UP:
      return ImGuiKey_UpArrow;
    case GLFW_KEY_DOWN:
      return ImGuiKey_DownArrow;
    case GLFW_KEY_PAGE_UP:
      return ImGuiKey_PageUp;
    case GLFW_KEY_PAGE_DOWN:
      return ImGuiKey_PageDown;
    case GLFW_KEY_HOME:
      return ImGuiKey_Home;
    case GLFW_KEY_END:
      return ImGuiKey_End;
    case GLFW_KEY_INSERT:
      return ImGuiKey_Insert;
    case GLFW_KEY_DELETE:
      return ImGuiKey_Delete;
    case GLFW_KEY_BACKSPACE:
      return ImGuiKey_Backspace;
    case GLFW_KEY_SPACE:
      return ImGuiKey_Space;
    case GLFW_KEY_ENTER:
      return ImGuiKey_Enter;
    case GLFW_KEY_ESCAPE:
      return ImGuiKey_Escape;
    case GLFW_KEY_APOSTROPHE:
      return ImGuiKey_Apostrophe;
    case GLFW_KEY_COMMA:
      return ImGuiKey_Comma;
    case GLFW_KEY_MINUS:
      return ImGuiKey_Minus;
    case GLFW_KEY_PERIOD:
      return ImGuiKey_Period;
    case GLFW_KEY_SLASH:
      return ImGuiKey_Slash;
    case GLFW_KEY_SEMICOLON:
      return ImGuiKey_Semicolon;
    case GLFW_KEY_EQUAL:
      return ImGuiKey_Equal;
    case GLFW_KEY_LEFT_BRACKET:
      return ImGuiKey_LeftBracket;
    case GLFW_KEY_BACKSLASH:
      return ImGuiKey_Backslash;
    case GLFW_KEY_RIGHT_BRACKET:
      return ImGuiKey_RightBracket;
    case GLFW_KEY_GRAVE_ACCENT:
      return ImGuiKey_GraveAccent;
    case GLFW_KEY_CAPS_LOCK:
      return ImGuiKey_CapsLock;
    case GLFW_KEY_SCROLL_LOCK:
      return ImGuiKey_ScrollLock;
    case GLFW_KEY_NUM_LOCK:
      return ImGuiKey_NumLock;
    case GLFW_KEY_PRINT_SCREEN:
      return ImGuiKey_PrintScreen;
    case GLFW_KEY_PAUSE:
      return ImGuiKey_Pause;
    case GLFW_KEY_KP_0:
      return ImGuiKey_Keypad0;
    case GLFW_KEY_KP_1:
      return ImGuiKey_Keypad1;
    case GLFW_KEY_KP_2:
      return ImGuiKey_Keypad2;
    case GLFW_KEY_KP_3:
      return ImGuiKey_Keypad3;
    case GLFW_KEY_KP_4:
      return ImGuiKey_Keypad4;
    case GLFW_KEY_KP_5:
      return ImGuiKey_Keypad5;
    case GLFW_KEY_KP_6:
      return ImGuiKey_Keypad6;
    case GLFW_KEY_KP_7:
      return ImGuiKey_Keypad7;
    case GLFW_KEY_KP_8:
      return ImGuiKey_Keypad8;
    case GLFW_KEY_KP_9:
      return ImGuiKey_Keypad9;
    case GLFW_KEY_KP_DECIMAL:
      return ImGuiKey_KeypadDecimal;
    case GLFW_KEY_KP_DIVIDE:
      return ImGuiKey_KeypadDivide;
    case GLFW_KEY_KP_MULTIPLY:
      return ImGuiKey_KeypadMultiply;
    case GLFW_KEY_KP_SUBTRACT:
      return ImGuiKey_KeypadSubtract;
    case GLFW_KEY_KP_ADD:
      return ImGuiKey_KeypadAdd;
    case GLFW_KEY_KP_ENTER:
      return ImGuiKey_KeypadEnter;
    case GLFW_KEY_KP_EQUAL:
      return ImGuiKey_KeypadEqual;
    case GLFW_KEY_LEFT_SHIFT:
      return ImGuiKey_LeftShift;
    case GLFW_KEY_LEFT_CONTROL:
      return ImGuiKey_LeftCtrl;
    case GLFW_KEY_LEFT_ALT:
      return ImGuiKey_LeftAlt;
    case GLFW_KEY_LEFT_SUPER:
      return ImGuiKey_LeftSuper;
    case GLFW_KEY_RIGHT_SHIFT:
      return ImGuiKey_RightShift;
    case GLFW_KEY_RIGHT_CONTROL:
      return ImGuiKey_RightCtrl;
    case GLFW_KEY_RIGHT_ALT:
      return ImGuiKey_RightAlt;
    case GLFW_KEY_RIGHT_SUPER:
      return ImGuiKey_RightSuper;
    case GLFW_KEY_MENU:
      return ImGuiKey_Menu;
    case GLFW_KEY_0:
      return ImGuiKey_0;
    case GLFW_KEY_1:
      return ImGuiKey_1;
    case GLFW_KEY_2:
      return ImGuiKey_2;
    case GLFW_KEY_3:
      return ImGuiKey_3;
    case GLFW_KEY_4:
      return ImGuiKey_4;
    case GLFW_KEY_5:
      return ImGuiKey_5;
    case GLFW_KEY_6:
      return ImGuiKey_6;
    case GLFW_KEY_7:
      return ImGuiKey_7;
    case GLFW_KEY_8:
      return ImGuiKey_8;
    case GLFW_KEY_9:
      return ImGuiKey_9;
    case GLFW_KEY_A:
      return ImGuiKey_A;
    case GLFW_KEY_B:
      return ImGuiKey_B;
    case GLFW_KEY_C:
      return ImGuiKey_C;
    case GLFW_KEY_D:
      return ImGuiKey_D;
    case GLFW_KEY_E:
      return ImGuiKey_E;
    case GLFW_KEY_F:
      return ImGuiKey_F;
    case GLFW_KEY_G:
      return ImGuiKey_G;
    case GLFW_KEY_H:
      return ImGuiKey_H;
    case GLFW_KEY_I:
      return ImGuiKey_I;
    case GLFW_KEY_J:
      return ImGuiKey_J;
    case GLFW_KEY_K:
      return ImGuiKey_K;
    case GLFW_KEY_L:
      return ImGuiKey_L;
    case GLFW_KEY_M:
      return ImGuiKey_M;
    case GLFW_KEY_N:
      return ImGuiKey_N;
    case GLFW_KEY_O:
      return ImGuiKey_O;
    case GLFW_KEY_P:
      return ImGuiKey_P;
    case GLFW_KEY_Q:
      return ImGuiKey_Q;
    case GLFW_KEY_R:
      return ImGuiKey_R;
    case GLFW_KEY_S:
      return ImGuiKey_S;
    case GLFW_KEY_T:
      return ImGuiKey_T;
    case GLFW_KEY_U:
      return ImGuiKey_U;
    case GLFW_KEY_V:
      return ImGuiKey_V;
    case GLFW_KEY_W:
      return ImGuiKey_W;
    case GLFW_KEY_X:
      return ImGuiKey_X;
    case GLFW_KEY_Y:
      return ImGuiKey_Y;
    case GLFW_KEY_Z:
      return ImGuiKey_Z;
    case GLFW_KEY_F1:
      return ImGuiKey_F1;
    case GLFW_KEY_F2:
      return ImGuiKey_F2;
    case GLFW_KEY_F3:
      return ImGuiKey_F3;
    case GLFW_KEY_F4:
      return ImGuiKey_F4;
    case GLFW_KEY_F5:
      return ImGuiKey_F5;
    case GLFW_KEY_F6:
      return ImGuiKey_F6;
    case GLFW_KEY_F7:
      return ImGuiKey_F7;
    case GLFW_KEY_F8:
      return ImGuiKey_F8;
    case GLFW_KEY_F9:
      return ImGuiKey_F9;
    case GLFW_KEY_F10:
      return ImGuiKey_F10;
    case GLFW_KEY_F11:
      return ImGuiKey_F11;
    case GLFW_KEY_F12:
      return ImGuiKey_F12;
    default:
      return ImGuiKey_None;
  }
}
ImGuiKey ImGuiInputAdapter::ConvertGlfwMouseButtonToImGuiKey(int glfwButton)
{
  switch (glfwButton) {
    case GLFW_MOUSE_BUTTON_LEFT:
      return ImGuiKey_MouseLeft;
    case GLFW_MOUSE_BUTTON_RIGHT:
      return ImGuiKey_MouseRight;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      return ImGuiKey_MouseMiddle;
    case GLFW_MOUSE_BUTTON_4:
      return ImGuiKey_MouseX1;
    case GLFW_MOUSE_BUTTON_5:
      return ImGuiKey_MouseX2;
    default:
      return ImGuiKey_None;
  }
}
}  // namespace mite