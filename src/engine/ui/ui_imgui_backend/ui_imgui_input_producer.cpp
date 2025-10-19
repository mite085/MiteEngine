#include "ui_imgui_input_producer.h"

namespace mite {

// 静态成员初始化
ImVec2 UIImguiInputProducer::s_LastMousePos = ImVec2(-1, -1);
bool UIImguiInputProducer::s_LastMouseButtons[ImGuiMouseButton_COUNT] = {false};
bool UIImguiInputProducer::s_LastKeys[ImGuiKey_COUNT] = {false};

void UIImguiInputProducer::ProduceInputEvents()
{
  // 获取ImGui输入状态
  ImGuiIO &io = ImGui::GetIO();

  // 按顺序生产各类输入事件
  ProduceMouseMoveEvents();
  ProduceMouseButtonEvents();
  ProduceKeyboardEvents();
  ProduceMouseScrollEvents();
}

void UIImguiInputProducer::ProduceMouseMoveEvents()
{
  ImGuiIO &io = ImGui::GetIO();

  // 只在鼠标位置变化时生成事件
  if (io.MousePos.x != s_LastMousePos.x || io.MousePos.y != s_LastMousePos.y) {
    EventBus::Publish<MouseMoveEvent>(MouseMoveEvent(io.MousePos.x, io.MousePos.y));
    s_LastMousePos = io.MousePos;
  }
}

void UIImguiInputProducer::ProduceMouseButtonEvents()
{
  ImGuiIO &io = ImGui::GetIO();

  // 遍历所有鼠标按钮
  for (int button = 0; button < ImGuiMouseButton_COUNT; button++) {
    bool currentState = io.MouseDown[button];
    bool lastState = s_LastMouseButtons[button];

    // 只在状态变化时生成事件
    if (currentState != lastState) {
      if (currentState) {
        // 鼠标按下事件
        EventBus::Publish<MouseButtonPressedEvent>(
            MouseButtonPressedEvent(button, 0, io.MousePos.x, io.MousePos.y));
      }
      else {
        // 鼠标释放事件
        EventBus::Publish<MouseButtonReleasedEvent>(
            MouseButtonReleasedEvent(button, io.MousePos.x, io.MousePos.y));
      }
      s_LastMouseButtons[button] = currentState;
    }
  }
}

void UIImguiInputProducer::ProduceKeyboardEvents()
{
  // 遍历所有命名按键（避免遍历整个键盘数组）
  for (int imguiKey = ImGuiKey_NamedKey_BEGIN; imguiKey < ImGuiKey_NamedKey_END; imguiKey++) {
    bool currentState = ImGui::IsKeyDown((ImGuiKey)imguiKey);
    bool lastState = s_LastKeys[imguiKey];

    // 只在状态变化时生成事件
    if (currentState != lastState) {
      int glfwKey = MapImGuiKeyToGLFW(imguiKey);
      if (glfwKey != GLFW_KEY_UNKNOWN) {
        if (currentState) {
          // 键盘按下事件（暂时不支持重复按键检测）
          EventBus::Publish<KeyPressedEvent>(KeyPressedEvent(glfwKey, 0, false));
        }
        else {
          // 键盘释放事件
          EventBus::Publish<KeyReleasedEvent>(KeyReleasedEvent(glfwKey));
        }
      }
      s_LastKeys[imguiKey] = currentState;
    }
  }
}

void UIImguiInputProducer::ProduceMouseScrollEvents()
{
  ImGuiIO &io = ImGui::GetIO();

  // 有滚轮偏移就生成事件
  if (io.MouseWheel != 0 || io.MouseWheelH != 0) {
    EventBus::Publish<MouseScrollEvent>(MouseScrollEvent(io.MouseWheelH, io.MouseWheel));

    // 注意：不需要重置io.MouseWheel和io.MouseWheelH
    // ImGui会在每帧开始时自动重置这些值，我们只需要在它们非零时生成事件即可
  }
}

int UIImguiInputProducer::MapImGuiKeyToGLFW(int imguiKey)
{
  // ImGuiKey到GLFW键码的转换（参考ImGui_ImplGlfw_KeyToImGuiKey）

  switch (imguiKey) {
    case ImGuiKey_Tab:
      return GLFW_KEY_TAB;
    case ImGuiKey_LeftArrow:
      return GLFW_KEY_LEFT;
    case ImGuiKey_RightArrow:
      return GLFW_KEY_RIGHT;
    case ImGuiKey_UpArrow:
      return GLFW_KEY_UP;
    case ImGuiKey_DownArrow:
      return GLFW_KEY_DOWN;
    case ImGuiKey_PageUp:
      return GLFW_KEY_PAGE_UP;
    case ImGuiKey_PageDown:
      return GLFW_KEY_PAGE_DOWN;
    case ImGuiKey_Home:
      return GLFW_KEY_HOME;
    case ImGuiKey_End:
      return GLFW_KEY_END;
    case ImGuiKey_Insert:
      return GLFW_KEY_INSERT;
    case ImGuiKey_Delete:
      return GLFW_KEY_DELETE;
    case ImGuiKey_Backspace:
      return GLFW_KEY_BACKSPACE;
    case ImGuiKey_Space:
      return GLFW_KEY_SPACE;
    case ImGuiKey_Enter:
      return GLFW_KEY_ENTER;
    case ImGuiKey_Escape:
      return GLFW_KEY_ESCAPE;
    case ImGuiKey_Apostrophe:
      return GLFW_KEY_APOSTROPHE;
    case ImGuiKey_Comma:
      return GLFW_KEY_COMMA;
    case ImGuiKey_Minus:
      return GLFW_KEY_MINUS;
    case ImGuiKey_Period:
      return GLFW_KEY_PERIOD;
    case ImGuiKey_Slash:
      return GLFW_KEY_SLASH;
    case ImGuiKey_Semicolon:
      return GLFW_KEY_SEMICOLON;
    case ImGuiKey_Equal:
      return GLFW_KEY_EQUAL;
    case ImGuiKey_LeftBracket:
      return GLFW_KEY_LEFT_BRACKET;
    case ImGuiKey_Backslash:
      return GLFW_KEY_BACKSLASH;
    case ImGuiKey_RightBracket:
      return GLFW_KEY_RIGHT_BRACKET;
    case ImGuiKey_GraveAccent:
      return GLFW_KEY_GRAVE_ACCENT;
    case ImGuiKey_CapsLock:
      return GLFW_KEY_CAPS_LOCK;
    case ImGuiKey_ScrollLock:
      return GLFW_KEY_SCROLL_LOCK;
    case ImGuiKey_NumLock:
      return GLFW_KEY_NUM_LOCK;
    case ImGuiKey_PrintScreen:
      return GLFW_KEY_PRINT_SCREEN;
    case ImGuiKey_Pause:
      return GLFW_KEY_PAUSE;
    case ImGuiKey_Keypad0:
      return GLFW_KEY_KP_0;
    case ImGuiKey_Keypad1:
      return GLFW_KEY_KP_1;
    case ImGuiKey_Keypad2:
      return GLFW_KEY_KP_2;
    case ImGuiKey_Keypad3:
      return GLFW_KEY_KP_3;
    case ImGuiKey_Keypad4:
      return GLFW_KEY_KP_4;
    case ImGuiKey_Keypad5:
      return GLFW_KEY_KP_5;
    case ImGuiKey_Keypad6:
      return GLFW_KEY_KP_6;
    case ImGuiKey_Keypad7:
      return GLFW_KEY_KP_7;
    case ImGuiKey_Keypad8:
      return GLFW_KEY_KP_8;
    case ImGuiKey_Keypad9:
      return GLFW_KEY_KP_9;
    case ImGuiKey_KeypadDecimal:
      return GLFW_KEY_KP_DECIMAL;
    case ImGuiKey_KeypadDivide:
      return GLFW_KEY_KP_DIVIDE;
    case ImGuiKey_KeypadMultiply:
      return GLFW_KEY_KP_MULTIPLY;
    case ImGuiKey_KeypadSubtract:
      return GLFW_KEY_KP_SUBTRACT;
    case ImGuiKey_KeypadAdd:
      return GLFW_KEY_KP_ADD;
    case ImGuiKey_KeypadEnter:
      return GLFW_KEY_KP_ENTER;
    case ImGuiKey_KeypadEqual:
      return GLFW_KEY_KP_EQUAL;
    case ImGuiKey_LeftShift:
      return GLFW_KEY_LEFT_SHIFT;
    case ImGuiKey_LeftCtrl:
      return GLFW_KEY_LEFT_CONTROL;
    case ImGuiKey_LeftAlt:
      return GLFW_KEY_LEFT_ALT;
    case ImGuiKey_LeftSuper:
      return GLFW_KEY_LEFT_SUPER;
    case ImGuiKey_RightShift:
      return GLFW_KEY_RIGHT_SHIFT;
    case ImGuiKey_RightCtrl:
      return GLFW_KEY_RIGHT_CONTROL;
    case ImGuiKey_RightAlt:
      return GLFW_KEY_RIGHT_ALT;
    case ImGuiKey_RightSuper:
      return GLFW_KEY_RIGHT_SUPER;
    case ImGuiKey_Menu:
      return GLFW_KEY_MENU;
    case ImGuiKey_0:
      return GLFW_KEY_0;
    case ImGuiKey_1:
      return GLFW_KEY_1;
    case ImGuiKey_2:
      return GLFW_KEY_2;
    case ImGuiKey_3:
      return GLFW_KEY_3;
    case ImGuiKey_4:
      return GLFW_KEY_4;
    case ImGuiKey_5:
      return GLFW_KEY_5;
    case ImGuiKey_6:
      return GLFW_KEY_6;
    case ImGuiKey_7:
      return GLFW_KEY_7;
    case ImGuiKey_8:
      return GLFW_KEY_8;
    case ImGuiKey_9:
      return GLFW_KEY_9;
    case ImGuiKey_A:
      return GLFW_KEY_A;
    case ImGuiKey_B:
      return GLFW_KEY_B;
    case ImGuiKey_C:
      return GLFW_KEY_C;
    case ImGuiKey_D:
      return GLFW_KEY_D;
    case ImGuiKey_E:
      return GLFW_KEY_E;
    case ImGuiKey_F:
      return GLFW_KEY_F;
    case ImGuiKey_G:
      return GLFW_KEY_G;
    case ImGuiKey_H:
      return GLFW_KEY_H;
    case ImGuiKey_I:
      return GLFW_KEY_I;
    case ImGuiKey_J:
      return GLFW_KEY_J;
    case ImGuiKey_K:
      return GLFW_KEY_K;
    case ImGuiKey_L:
      return GLFW_KEY_L;
    case ImGuiKey_M:
      return GLFW_KEY_M;
    case ImGuiKey_N:
      return GLFW_KEY_N;
    case ImGuiKey_O:
      return GLFW_KEY_O;
    case ImGuiKey_P:
      return GLFW_KEY_P;
    case ImGuiKey_Q:
      return GLFW_KEY_Q;
    case ImGuiKey_R:
      return GLFW_KEY_R;
    case ImGuiKey_S:
      return GLFW_KEY_S;
    case ImGuiKey_T:
      return GLFW_KEY_T;
    case ImGuiKey_U:
      return GLFW_KEY_U;
    case ImGuiKey_V:
      return GLFW_KEY_V;
    case ImGuiKey_W:
      return GLFW_KEY_W;
    case ImGuiKey_X:
      return GLFW_KEY_X;
    case ImGuiKey_Y:
      return GLFW_KEY_Y;
    case ImGuiKey_Z:
      return GLFW_KEY_Z;
    case ImGuiKey_F1:
      return GLFW_KEY_F1;
    case ImGuiKey_F2:
      return GLFW_KEY_F2;
    case ImGuiKey_F3:
      return GLFW_KEY_F3;
    case ImGuiKey_F4:
      return GLFW_KEY_F4;
    case ImGuiKey_F5:
      return GLFW_KEY_F5;
    case ImGuiKey_F6:
      return GLFW_KEY_F6;
    case ImGuiKey_F7:
      return GLFW_KEY_F7;
    case ImGuiKey_F8:
      return GLFW_KEY_F8;
    case ImGuiKey_F9:
      return GLFW_KEY_F9;
    case ImGuiKey_F10:
      return GLFW_KEY_F10;
    case ImGuiKey_F11:
      return GLFW_KEY_F11;
    case ImGuiKey_F12:
      return GLFW_KEY_F12;
    case ImGuiKey_F13:
      return GLFW_KEY_F13;
    case ImGuiKey_F14:
      return GLFW_KEY_F14;
    case ImGuiKey_F15:
      return GLFW_KEY_F15;
    case ImGuiKey_F16:
      return GLFW_KEY_F16;
    case ImGuiKey_F17:
      return GLFW_KEY_F17;
    case ImGuiKey_F18:
      return GLFW_KEY_F18;
    case ImGuiKey_F19:
      return GLFW_KEY_F19;
    case ImGuiKey_F20:
      return GLFW_KEY_F20;
    case ImGuiKey_F21:
      return GLFW_KEY_F21;
    case ImGuiKey_F22:
      return GLFW_KEY_F22;
    case ImGuiKey_F23:
      return GLFW_KEY_F23;
    case ImGuiKey_F24:
      return GLFW_KEY_F24;
    case ImGuiKey_Oem102:
      return GLFW_KEY_WORLD_1;  // 注意：GLFW_KEY_WORLD_1 和 GLFW_KEY_WORLD_2 都映射到
                                // ImGuiKey_Oem102
    default:
      return GLFW_KEY_UNKNOWN;
  }
}

}  // namespace mite
