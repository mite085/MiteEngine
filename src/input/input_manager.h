#ifndef MITE_INPUT_MANAGER
#define MITE_INPUT_MANAGER

#include "headers.h"
#include "input_context.h"

namespace mite {

// InputManager������ģ��ĺ���ʵ���࣬
// ����ͳһ�������������豸��״̬��
// ���������¼��ķַ��߼�����ά������������ջ��
class InputManager {
 public:
  static void Init(const std::shared_ptr<InputContextStack> &stack);
  static void Shutdown();
  static void Update();

  // �豸״̬
  static void SetKeyState(KeyCode key, InputState state);
  static void SetMouseButtonState(MouseCode button, InputState state);
  static void SetMousePosition(const glm::vec2 &position);
  static void SetMouseScrollDelta(float delta);

  // �����Ĺ���
  static void PushContext(std::shared_ptr<InputContext> context);
  static void PopContext();
  static std::shared_ptr<InputContext> GetCurrentContext();

  // ������ѯ
  static float GetActionValue(const std::string &actionName);

  // ԭʼ�����ѯ--Key
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
  // ����״̬ (��ǰ֡����һ֡)
  // GLFW_KEY_LAST=348��
  // �ʴ˴�����һ����������ƣ�
  // ��array��size��Ϊ512
  static std::array<InputState, 512> s_KeyStates;
  static std::array<InputState, 512> s_PrevKeyStates;
  static std::array<float, 512> s_KeyHeldDurations;

  // ���״̬
  // GLFW_MOUSE_BUTTON_1��
  // ��GLFW_MOUSE_BUTTON_8
  // �����1�����Ҽ���2�����м���3��+ �����4-8��
  static std::array<InputState, 8> s_MouseButtonStates;
  static std::array<InputState, 8> s_PrevMouseButtonStates;
  static glm::vec2 s_MousePosition;
  static glm::vec2 s_PrevMousePosition;
  static glm::vec2 s_MouseDelta;
  static float s_MouseScrollDelta;
  static float s_PrevMouseScrollDelta;

  // ����������ջ
  static std::shared_ptr<InputContextStack> s_ContextStack;
};

};

#endif
