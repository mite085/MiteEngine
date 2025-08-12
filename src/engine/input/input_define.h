#ifndef MITE_INPUT_DEFINE
#define MITE_INPUT_DEFINE

namespace mite {

// 输入设备与状态枚举定义
enum class InputDevice { Keyboard, Mouse };
enum class InputState { Released, Pressed, Held, Repeated };

// 输入键码别名
using KeyCode = int;
using MouseCode = int;

};

#endif
