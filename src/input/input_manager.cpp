#include "input_manager.h"

namespace mite {

// ��̬��Ա��ʼ��
std::array<InputState, 512> InputManager::s_KeyStates;
std::array<InputState, 512> InputManager::s_PrevKeyStates;
std::array<float, 512> InputManager::s_KeyHeldDurations;
std::array<InputState, 8> InputManager::s_MouseButtonStates;
std::array<InputState, 8> InputManager::s_PrevMouseButtonStates;
glm::vec2 InputManager::s_MousePosition;
glm::vec2 InputManager::s_PrevMousePosition;
glm::vec2 InputManager::s_MouseDelta;
float InputManager::s_MouseScrollDelta;
float InputManager::s_PrevMouseScrollDelta;
std::shared_ptr<InputContextStack> InputManager::s_ContextStack;

void InputManager::Init(const std::shared_ptr<InputContextStack> &stack)
{
  // ��Application��������������ջ������Manager���й���
  s_ContextStack = stack;

  // ��ʼ�����м�״̬ΪReleased
  s_KeyStates.fill(InputState::Released);
  s_PrevKeyStates.fill(InputState::Released);
  s_KeyHeldDurations.fill(0.0f);

  // ��ʼ����갴ť״̬
  s_MouseButtonStates.fill(InputState::Released);
  s_PrevMouseButtonStates.fill(InputState::Released);

  // ��ʼ�����λ�ú͹���
  s_MousePosition = glm::vec2(0.0f);
  s_PrevMousePosition = glm::vec2(0.0f);
  s_MouseDelta = glm::vec2(0.0f);
  s_MouseScrollDelta = 0.0f;
  s_PrevMouseScrollDelta = 0.0f;

  // ȷ��������һ��Ĭ��������
  if (s_ContextStack->IsEmpty()) {
    auto defaultContext = std::make_shared<InputContext>("Default");
    s_ContextStack->Push(defaultContext);
  }
}
void InputManager::Shutdown()
{  // ��������������
  s_ContextStack->Clear();

  // ������������״̬
  s_KeyStates.fill(InputState::Released);
  s_PrevKeyStates.fill(InputState::Released);
  s_KeyHeldDurations.fill(0.0f);
  s_MouseButtonStates.fill(InputState::Released);
  s_PrevMouseButtonStates.fill(InputState::Released);
}
void InputManager::Update()
{
  // ���°�������ʱ�� - ����ʹ��Time::DeltaTime()
  for (size_t i = 0; i < s_KeyStates.size(); ++i) {
    if (s_KeyStates[i] == InputState::Pressed || s_KeyStates[i] == InputState::Held ||
        s_KeyStates[i] == InputState::Repeated)
    {
      s_KeyHeldDurations[i] += Time::DeltaTime();

      // ��Pressed״̬תΪHeld״̬
      if (s_KeyStates[i] == InputState::Pressed) {
        s_KeyStates[i] = InputState::Held;
      }
    }
    else {
      s_KeyHeldDurations[i] = 0.0f;
    }
  }

  // ������갴ť״̬
  for (size_t i = 0; i < s_MouseButtonStates.size(); ++i) {
    if (s_MouseButtonStates[i] == InputState::Pressed) {
      s_MouseButtonStates[i] = InputState::Held;
    }
  }

  // ����ƶ������״̬��GLFW�ص��ı䣨�첽��������inputģ�鸺��
  // s_MousePosition =

  // ����Delta������ƶ���������ֱ仯����
  s_MouseDelta = s_MousePosition - s_PrevMousePosition;
  s_MouseScrollDelta = s_MouseScrollDelta - s_PrevMouseScrollDelta;

  // ȫ��������Ϻ󣬸�����һ֡״̬Ϊ��ǰ״̬��
  s_PrevKeyStates = s_KeyStates;
  s_PrevMouseButtonStates = s_MouseButtonStates;
  s_PrevMousePosition = s_MousePosition;
  s_PrevMouseScrollDelta = s_MouseScrollDelta;

  // Mainloop���һ��ѭ�����ص�InputManager::Update()��
  // ������һ�ֵ�Key��Mouse״̬��ѯ�����
}

void InputManager::SetKeyState(KeyCode key, InputState state)
{
  // ȷ����������Ч��Χ��
  if (key < 0 || key >= s_KeyStates.size()) {
    return;
  }

  // ���°���״̬
  InputState prevState = s_KeyStates[key];
  s_KeyStates[key] = state;

  // ����״̬ת��
  if (state == InputState::Pressed) {
    // �����ոհ��£����ó���ʱ��
    s_KeyHeldDurations[key] = 0.0f;
  }
  else if (state == InputState::Held && prevState == InputState::Pressed) {
    // ��Pressedת��ΪHeld״̬
    s_KeyStates[key] = InputState::Held;
  }
}

void InputManager::SetMouseButtonState(MouseCode button, InputState state)
{
  // ȷ����갴ť������Ч��Χ��
  if (button < 0 || button >= s_MouseButtonStates.size()) {
    return;
  }

  // ������갴ť״̬
  InputState prevState = s_MouseButtonStates[button];
  s_MouseButtonStates[button] = state;

  // ����״̬ת��
  if (state == InputState::Pressed) {
    // ��ť�ոհ���
  }
  else if (state == InputState::Held && prevState == InputState::Pressed) {
    // ��Pressedת��ΪHeld״̬
    s_MouseButtonStates[button] = InputState::Held;
  }
}

void InputManager::SetMousePosition(const glm::vec2 &position)
{
  // �������λ��
  s_PrevMousePosition = s_MousePosition;
  s_MousePosition = position;

  // ��������ƶ�����
  s_MouseDelta = s_MousePosition - s_PrevMousePosition;
}

void InputManager::SetMouseScrollDelta(float delta)
{
  // ��������������
  s_PrevMouseScrollDelta = s_MouseScrollDelta;
  s_MouseScrollDelta = delta;
}

void InputManager::PushContext(std::shared_ptr<InputContext> context)
{
  if (!context) {
    LOG_ERROR("Attempted to push null input context");
    return;
  }

  // �ǿ�ջ����£�
  // ��־��¼Input�������ջ˳��
  if (!s_ContextStack->IsEmpty()) {
    auto &current = s_ContextStack->GetCurrent();
    LOG_DEBUG("Pushing input context '{}' over '{}'", context->GetName(), current->GetName());
  }
  else {
    LOG_DEBUG("Pushing first input context '{}'", context->GetName());
  }

  // ִ����ջ����
  s_ContextStack->Push(context);
}

void InputManager::PopContext()
{
  if (s_ContextStack->IsEmpty()) {
    LOG_ERROR("Attempted to pop empty input context stack");
    return;
  }

  // ִ�г�ջ����
  auto popped = s_ContextStack->GetCurrent();
  s_ContextStack->Pop();

  // ��ջ��ǿ�ջ������£�
  // ��־��¼��һ��Input�¼�
  if (!s_ContextStack->IsEmpty()) {
    auto &newCurrent = s_ContextStack->GetCurrent();
    LOG_DEBUG("Popped input context '{}', new current is '{}'",
              popped->GetName(),
              newCurrent->GetName());
  }
  else {
    LOG_DEBUG("Popped last input context '{}'", popped->GetName());
  }
}

std::shared_ptr<InputContext> InputManager::GetCurrentContext()
{
  // ��ջ����·��ʵ�ǰ������
  if (s_ContextStack->IsEmpty()) {
    LOG_WARN("No input context available");
    return nullptr;
  }

  return s_ContextStack->GetCurrent();
}

float InputManager::GetActionValue(const std::string &actionName)
{
  // ���û�л�Ծ�����������ģ�����0.0
  if (s_ContextStack->IsEmpty())
    return 0.0f;

  // ��ȡ��ǰ������
  auto &currentContext = s_ContextStack->GetCurrent();

  // ����ָ�����ƵĶ���
  InputAction *action = currentContext->GetAction(actionName);
  if (!action)
    return 0.0f;

  // ���ö���ֵ
  action->value = 0.0f;

  // �������������а�
  for (const auto &binding : action->bindings) {
    switch (binding.device) {
      case InputDevice::Keyboard: {
        // ����Ǽ��̰��Ұ��������£��ۼ�����ֵ
        if (GetKeyState(binding.code) == InputState::Pressed ||
            GetKeyState(binding.code) == InputState::Held ||
            GetKeyState(binding.code) == InputState::Repeated)
        {
          action->value += binding.scale;
        }
        break;
      }
      case InputDevice::Mouse: {
        // �������갴ť���Ұ�ť�����£��ۼ�����ֵ
        if (GetMouseButtonState(binding.code) == InputState::Pressed ||
            GetMouseButtonState(binding.code) == InputState::Held ||
            GetMouseButtonState(binding.code) == InputState::Repeated)
        {
          action->value += binding.scale;
        }
        break;
      }
    }
  }

  // ���ض���������ֵ��������-1.0��1.0֮�䣩
  return glm::clamp(action->value, -1.0f, 1.0f);
}

InputState InputManager::GetKeyState(KeyCode key)
{
  if (key >= 0 && key < s_KeyStates.size()) {
    return s_KeyStates[key];
  }
  return InputState::Released;
}

InputState InputManager::GetPrevKeyState(KeyCode key)
{
  if (key >= 0 && key < s_PrevKeyStates.size()) {
    return s_PrevKeyStates[key];
  }
  return InputState::Released;
}

float InputManager::GetKeyHeldDuration(KeyCode key)
{
  if (key >= 0 && key < s_KeyHeldDurations.size()) {
    return s_KeyHeldDurations[key];
  }
  return 0.0f;
}

InputState InputManager::GetMouseButtonState(MouseCode button)
{
  // �����갴ť���Ƿ�����Ч��Χ��
  if (button >= 0 && button < s_MouseButtonStates.size()) {
    // ���ص�ǰ֡����갴ť״̬��Pressed �� Held ����Ϊ���£�
    return s_MouseButtonStates[button];
  }
  return InputState::Released;
}

InputState InputManager::GetPrevMouseButtonState(MouseCode button)
{
  // �����һ֡��갴ť���Ƿ�����Ч��Χ��
  if (button >= 0 && button < s_PrevMouseButtonStates.size()) {
    // ������һ֡����갴ť״̬��Pressed �� Held ����Ϊ���£�
    return s_PrevMouseButtonStates[button];
  }
  return InputState::Released;
}

glm::vec2 InputManager::GetMousePosition()
{
  return s_MousePosition;
}

glm::vec2 InputManager::GetMouseDelta()
{
  return s_MouseDelta;
}

float InputManager::GetMouseScrollDelta(MouseCode button)
{
  return s_MouseScrollDelta;
}
};  // namespace mite