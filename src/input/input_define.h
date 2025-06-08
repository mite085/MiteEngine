#ifndef MITE_INPUT_DEFINE
#define MITE_INPUT_DEFINE

namespace mite {

// �����豸��״̬ö�ٶ���
enum class InputDevice { Keyboard, Mouse };
enum class InputState { Released, Pressed, Held, Repeated };

// ����������
using KeyCode = int;
using MouseCode = int;

};

#endif
