#ifndef MITE_INPUT
#define MITE_INPUT

#include "event/event.h"
#include "glm/glm.hpp"
#include "headers.h"

namespace mite {

// 输入设备与状态枚举定义
enum class InputDevice { Keyboard, Mouse };
enum class InputState { Released, Pressed, Held, Repeated };

// 输入键码别名
using KeyCode = int;
using MouseCode = int;

class Input {
 public:
  static void Init();
  static void Shutdown();
  static void Update();

  // 键盘状态查询
  static bool IsKeyPressed(KeyCode key);
  static bool IsKeyReleased(KeyCode key);
  static bool IsKeyHeld(KeyCode key);
  static float GetKeyHeldDuration(KeyCode key);

  // 鼠标状态查询
  static bool IsMouseButtonPressed(MouseCode button);
  static bool IsMouseButtonReleased(MouseCode button);
  static bool IsMouseButtonHeld(MouseCode button);
  static glm::vec2 GetMousePosition();
  static glm::vec2 GetMouseDelta();
  static float GetMouseScrollDelta(MouseCode button);

  // 输入上下文管理
  static void PushContext(const std::string &contextName);
  static void PopContext();
  static bool IsCurrentContext(const std::string &contextName);

  // TODO: 事件回调
  static void OnEvent(Event &event);
};


}  // namespace mite

#endif
