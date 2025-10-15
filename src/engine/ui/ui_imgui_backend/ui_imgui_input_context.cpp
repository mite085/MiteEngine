#include "ui_imgui_input_context.h"

namespace mite {
ImGuiInputContext::ImGuiInputContext() : InputContext("ImGUI Input Context")
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Input Context");
  m_Logger->info("Creating ImGuiInputContext");
}

ImGuiInputContext::~ImGuiInputContext()
{
  m_Logger->info("Destroying ImGuiInputContext");
}

void ImGuiInputContext::UpdateDisplaySize(GLFWwindow *window)
{
  if (!window) {
    m_Logger->warn("GLFW window not available, cannot update display size");
    return;
  }
  int width, height;

  // 获取窗口尺寸（逻辑分辨率）
  glfwGetWindowSize(window, &width, &height);
  m_DisplaySize = glm::ivec2(width, height);

  // 更新ImGui的显示尺寸
  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(m_DisplaySize.x),
                          static_cast<float>(m_DisplaySize.y));

  // m_Logger->debug("Display size updated: {}x{}", width, height);
}

void ImGuiInputContext::UpdateFramebufferScale(GLFWwindow *window)
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
  // m_Logger->debug(
  //     "Framebuffer scale updated: {:.2f}x{:.2f}", m_FramebufferScale.x, m_FramebufferScale.y);
}

void ImGuiInputContext::ProcessMouseMoveEvent(MouseMoveEvent &e)
{
  // 先检查是否应该捕获鼠标事件
  if (!ImGui::GetIO().WantCaptureMouse) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGui::GetIO().AddMousePosEvent(static_cast<float>(e.GetXPos()),
                                  static_cast<float>(e.GetYPos()));
  m_LastMousePos = glm::vec2(e.GetXPos(), e.GetYPos());

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e)
{
  // 先检查是否应该捕获鼠标事件
  if (!ImGui::GetIO().WantCaptureMouse) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGuiKey mouseKey = ConvertGlfwMouseButtonToImGuiKey(e.GetButton());
  if (mouseKey != ImGuiKey_None) {
    ImGui::GetIO().AddKeyEvent(mouseKey, true);
  }

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e)
{
  // 先检查是否应该捕获鼠标事件
  if (!ImGui::GetIO().WantCaptureMouse) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGuiKey mouseKey = ConvertGlfwMouseButtonToImGuiKey(e.GetButton());
  if (mouseKey != ImGuiKey_None) {
    ImGui::GetIO().AddKeyEvent(mouseKey, false);
  }

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessMouseScrollEvent(MouseScrollEvent &e)
{
  // 先检查是否应该捕获鼠标事件
  if (!ImGui::GetIO().WantCaptureMouse) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseWheelEvent(static_cast<float>(e.GetXOffset()), static_cast<float>(e.GetYOffset()));

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessKeyPressdEvent(KeyPressedEvent &e)
{
  // 先检查是否应该捕获键盘事件
  if (!ImGui::GetIO().WantCaptureKeyboard) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
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

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessKeyReleasedEvent(KeyReleasedEvent &e)
{
  // 先检查是否应该捕获键盘事件
  if (!ImGui::GetIO().WantCaptureKeyboard) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGuiKey key = ConvertGlfwKeyToImGuiKey(e.GetKey());
  if (key != ImGuiKey_None) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(key, false);
  }

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

void ImGuiInputContext::ProcessKeyTypedEvent(KeyTypedEvent &e)
{
  // 先检查是否应该捕获键盘事件
  if (!ImGui::GetIO().WantCaptureKeyboard) {
    return;  // 不处理，让事件继续传播
  }

  // ImGui捕获并消费事件
  ImGuiIO &io = ImGui::GetIO();
  io.AddInputCharacter(e.GetCodepoint());

  // 组织事件继续传播
  e.SetResult(EventResult::HandledAndStop);
}

// GLFW键码到ImGuiKey的转换（参考ImGui_ImplGlfw_KeyToImGuiKey）
// （由于ImGui_ImplGlfw_KeyToImGuiKey未在头文件声明，此处直接复制代码）
ImGuiKey ImGuiInputContext::ConvertGlfwKeyToImGuiKey(int glfwKey)
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
    case GLFW_KEY_WORLD_1:
      return ImGuiKey_Oem102;
    case GLFW_KEY_WORLD_2:
      return ImGuiKey_Oem102;
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
    case GLFW_KEY_F13:
      return ImGuiKey_F13;
    case GLFW_KEY_F14:
      return ImGuiKey_F14;
    case GLFW_KEY_F15:
      return ImGuiKey_F15;
    case GLFW_KEY_F16:
      return ImGuiKey_F16;
    case GLFW_KEY_F17:
      return ImGuiKey_F17;
    case GLFW_KEY_F18:
      return ImGuiKey_F18;
    case GLFW_KEY_F19:
      return ImGuiKey_F19;
    case GLFW_KEY_F20:
      return ImGuiKey_F20;
    case GLFW_KEY_F21:
      return ImGuiKey_F21;
    case GLFW_KEY_F22:
      return ImGuiKey_F22;
    case GLFW_KEY_F23:
      return ImGuiKey_F23;
    case GLFW_KEY_F24:
      return ImGuiKey_F24;
    default:
      return ImGuiKey_None;
  }
}

ImGuiKey ImGuiInputContext::ConvertGlfwMouseButtonToImGuiKey(int glfwButton)
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