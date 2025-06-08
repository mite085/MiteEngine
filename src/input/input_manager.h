#ifndef MITE_INPUT_MANAGER
#define MITE_INPUT_MANAGER

#include "headers.h"
#include "input_context.h"

namespace mite {

// InputManager是输入模块的核心实现类，
// 负责统一管理所有输入设备的状态、
// 处理输入事件的分发逻辑，并维护输入上下文栈。
class InputManager {
 public:
  static void Init(const std::shared_ptr<InputContextStack> &stack);
  static void Shutdown();
  static void Update();

  // 设备状态
  static void SetKeyState(KeyCode key, InputState state);
  static void SetMouseButtonState(MouseCode button, InputState state);
  static void SetMousePosition(const glm::vec2 &position);
  static void SetMouseScrollDelta(float delta);

  // 上下文管理
  static void PushContext(std::shared_ptr<InputContext> context);
  static void PopContext();
  static std::shared_ptr<InputContext> GetCurrentContext();

  // 动作查询
  static float GetActionValue(const std::string &actionName);

  // 原始输入查询--Key
  static InputState GetKeyState(KeyCode key);
  static InputState GetPrevKeyState(KeyCode key);
  static float GetKeyHeldDuration(KeyCode key); 
  // Mouse
  static InputState GetMouseButtonState(MouseCode button);
  static InputState GetPrevMouseButtonState(MouseCode button);
  static glm::vec2 GetMousePosition();
  static glm::vec2 GetMouseDelta();
  static float GetMouseScrollDelta(MouseCode button);

 private:
  // 键盘状态 (当前帧和上一帧)
  // GLFW_KEY_LAST=348，
  // 故此处考虑一定的冗余设计，
  // 将array的size订为512
  static std::array<InputState, 512> s_KeyStates;
  static std::array<InputState, 512> s_PrevKeyStates;
  static std::array<float, 512> s_KeyHeldDurations;

  // 鼠标状态
  // GLFW_MOUSE_BUTTON_1，
  // 到GLFW_MOUSE_BUTTON_8
  // 左键（1）、右键（2）、中键（3）+ 侧键（4-8）
  static std::array<InputState, 8> s_MouseButtonStates;
  static std::array<InputState, 8> s_PrevMouseButtonStates;
  static glm::vec2 s_MousePosition;
  static glm::vec2 s_PrevMousePosition;
  static glm::vec2 s_MouseDelta;
  static float s_MouseScrollDelta;
  static float s_PrevMouseScrollDelta;

  // 输入上下文栈
  static std::shared_ptr<InputContextStack> s_ContextStack;
};

};

#endif
