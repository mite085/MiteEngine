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
  // 当前帧按下且上一帧未按下
  auto currentState = InputManager::GetKeyState(key);
  auto prevState = InputManager::GetPrevKeyState(key);
  return currentState == InputState::Pressed && prevState != InputState::Pressed;
}

bool Input::IsKeyReleased(KeyCode key)
{
  // 当前帧释放且上一帧是按下状态
  auto currentState = InputManager::GetKeyState(key);
  auto prevState = InputManager::GetPrevKeyState(key);
  return currentState == InputState::Released && prevState == InputState::Pressed;
}

bool Input::IsKeyHeld(KeyCode key)
{
  // 当前帧处于按下状态（包括重复）
  auto currentState = InputManager::GetKeyState(key);
  return currentState == InputState::Pressed || currentState == InputState::Held ||
         currentState == InputState::Repeated;
}

float Input::GetKeyHeldDuration(KeyCode key)
{
  // 从InputManager获取按键持续按下的时间
  return InputManager::GetKeyHeldDuration(key);
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
  // 当前帧按下且上一帧未按下
  return InputManager::GetMouseButtonState(button) == InputState::Pressed &&
         InputManager::GetPrevMouseButtonState(button) != InputState::Pressed;
}

bool Input::IsMouseButtonReleased(MouseCode button)
{
  // 当前帧释放且上一帧是按下状态
  return InputManager::GetMouseButtonState(button) == InputState::Released &&
         InputManager::GetPrevMouseButtonState(button) == InputState::Pressed;
}

bool Input::IsMouseButtonHeld(MouseCode button)
{
  // 当前帧是按下状态
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
  // 从管理器栈中弹出顶部上下文
  InputManager::PopContext();
}

bool Input::IsCurrentContext(const std::string &contextName)
{
  // 获取当前上下文并比较名称
  std::shared_ptr<InputContext> current = InputManager::GetCurrentContext();
  return current && current->GetName() == contextName;
}

void Input::OnEvent(Event &event) {}
}  // namespace mite