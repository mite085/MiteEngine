#include "input.h"
#include "input_manager.h"

namespace mite {
void Input::Init(const std::shared_ptr<InputContextStack> &stack)
{
  InputManager::Init(stack);
}

void Input::Shutdown()
{
  InputManager::Shutdown();
}

void Input::Update()
{
  InputManager::Update();
}

bool Input::IsKeyPressed(KeyCode key)
{
  // ��ǰ֡��������һ֡δ����
  auto currentState = InputManager::GetKeyState(key);
  auto prevState = InputManager::GetPrevKeyState(key);
  return currentState == InputState::Pressed && prevState != InputState::Pressed;
}

bool Input::IsKeyReleased(KeyCode key)
{
  // ��ǰ֡�ͷ�����һ֡�ǰ���״̬
  auto currentState = InputManager::GetKeyState(key);
  auto prevState = InputManager::GetPrevKeyState(key);
  return currentState == InputState::Released && prevState == InputState::Pressed;
}

bool Input::IsKeyHeld(KeyCode key)
{
  // ��ǰ֡���ڰ���״̬�������ظ���
  auto currentState = InputManager::GetKeyState(key);
  return currentState == InputState::Pressed || currentState == InputState::Held ||
         currentState == InputState::Repeated;
}

float Input::GetKeyHeldDuration(KeyCode key)
{
  // ��InputManager��ȡ�����������µ�ʱ��
  return InputManager::GetKeyHeldDuration(key);
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
  // ��ǰ֡��������һ֡δ����
  return InputManager::GetMouseButtonState(button) == InputState::Pressed &&
         InputManager::GetPrevMouseButtonState(button) != InputState::Pressed;
}

bool Input::IsMouseButtonReleased(MouseCode button)
{
  // ��ǰ֡�ͷ�����һ֡�ǰ���״̬
  return InputManager::GetMouseButtonState(button) == InputState::Released &&
         InputManager::GetPrevMouseButtonState(button) == InputState::Pressed;
}

bool Input::IsMouseButtonHeld(MouseCode button)
{
  // ��ǰ֡�ǰ���״̬
  return InputManager::GetMouseButtonState(button) == InputState::Pressed ||
         InputManager::GetMouseButtonState(button) == InputState::Held;
}

glm::vec2 Input::GetMousePosition()
{
  return InputManager::GetMousePosition();
}

glm::vec2 Input::GetMouseDelta()
{
  return InputManager::GetMouseDelta();
}

float Input::GetMouseScrollDelta(MouseCode button)
{
  return InputManager::GetMouseScrollDelta(button);
}

void Input::PushContext(std::shared_ptr<InputContext> context)
{
  InputManager::PushContext(context);
}

void Input::PopContext()
{
  // �ӹ�����ջ�е�������������
  InputManager::PopContext();
}

bool Input::IsCurrentContext(const std::string &contextName)
{
  // ��ȡ��ǰ�����Ĳ��Ƚ�����
  std::shared_ptr<InputContext> current = InputManager::GetCurrentContext();
  return current && current->GetName() == contextName;
}

void Input::OnEvent(Event &event) {}
}  // namespace mite