#include "input_manager.h"

namespace mite {

// 静态成员初始化
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
  // 由Application创建输入上下文栈，并由Manager进行管理
  s_ContextStack = stack;

  // 初始化所有键状态为Released
  s_KeyStates.fill(InputState::Released);
  s_PrevKeyStates.fill(InputState::Released);
  s_KeyHeldDurations.fill(0.0f);

  // 初始化鼠标按钮状态
  s_MouseButtonStates.fill(InputState::Released);
  s_PrevMouseButtonStates.fill(InputState::Released);

  // 初始化鼠标位置和滚轮
  s_MousePosition = glm::vec2(0.0f);
  s_PrevMousePosition = glm::vec2(0.0f);
  s_MouseDelta = glm::vec2(0.0f);
  s_MouseScrollDelta = 0.0f;
  s_PrevMouseScrollDelta = 0.0f;

  // 确保至少有一个默认上下文
  if (s_ContextStack->IsEmpty()) {
    auto defaultContext = std::make_shared<InputContext>("Default");
    s_ContextStack->Push(defaultContext);
  }
}
void InputManager::Shutdown()
{  // 清理所有上下文
  s_ContextStack->Clear();

  // 重置所有输入状态
  s_KeyStates.fill(InputState::Released);
  s_PrevKeyStates.fill(InputState::Released);
  s_KeyHeldDurations.fill(0.0f);
  s_MouseButtonStates.fill(InputState::Released);
  s_PrevMouseButtonStates.fill(InputState::Released);
}
void InputManager::Update()
{
  // 更新按键持续时间 - 现在使用Time::DeltaTime()
  for (size_t i = 0; i < s_KeyStates.size(); ++i) {
    if (s_KeyStates[i] == InputState::Pressed || s_KeyStates[i] == InputState::Held ||
        s_KeyStates[i] == InputState::Repeated)
    {
      s_KeyHeldDurations[i] += Time::DeltaTime();

      // 将Pressed状态转为Held状态
      if (s_KeyStates[i] == InputState::Pressed) {
        s_KeyStates[i] = InputState::Held;
      }
    }
    else {
      s_KeyHeldDurations[i] = 0.0f;
    }
  }

  // 更新鼠标按钮状态
  for (size_t i = 0; i < s_MouseButtonStates.size(); ++i) {
    if (s_MouseButtonStates[i] == InputState::Pressed) {
      s_MouseButtonStates[i] = InputState::Held;
    }
  }

  // 鼠标移动与滚轮状态由GLFW回调改变（异步），并非input模块负责
  // s_MousePosition =

  // 计算Delta（鼠标移动增量与滚轮变化量）
  s_MouseDelta = s_MousePosition - s_PrevMousePosition;
  s_MouseScrollDelta = s_MouseScrollDelta - s_PrevMouseScrollDelta;

  // 全部计算完毕后，更新上一帧状态为当前状态。
  s_PrevKeyStates = s_KeyStates;
  s_PrevMouseButtonStates = s_MouseButtonStates;
  s_PrevMousePosition = s_MousePosition;
  s_PrevMouseScrollDelta = s_MouseScrollDelta;

  // Mainloop完成一次循环，回到InputManager::Update()后，
  // 开启新一轮的Key和Mouse状态查询与更新
}

void InputManager::SetKeyState(KeyCode key, InputState state)
{
  // 确保键码在有效范围内
  if (key < 0 || key >= s_KeyStates.size()) {
    return;
  }

  // 更新按键状态
  InputState prevState = s_KeyStates[key];
  s_KeyStates[key] = state;

  // 处理状态转换
  if (state == InputState::Pressed) {
    // 按键刚刚按下，重置持续时间
    s_KeyHeldDurations[key] = 0.0f;
  }
  else if (state == InputState::Held && prevState == InputState::Pressed) {
    // 从Pressed转换为Held状态
    s_KeyStates[key] = InputState::Held;
  }
}

void InputManager::SetMouseButtonState(MouseCode button, InputState state)
{
  // 确保鼠标按钮码在有效范围内
  if (button < 0 || button >= s_MouseButtonStates.size()) {
    return;
  }

  // 更新鼠标按钮状态
  InputState prevState = s_MouseButtonStates[button];
  s_MouseButtonStates[button] = state;

  // 处理状态转换
  if (state == InputState::Pressed) {
    // 按钮刚刚按下
  }
  else if (state == InputState::Held && prevState == InputState::Pressed) {
    // 从Pressed转换为Held状态
    s_MouseButtonStates[button] = InputState::Held;
  }
}

void InputManager::SetMousePosition(const glm::vec2 &position)
{
  // 更新鼠标位置
  s_PrevMousePosition = s_MousePosition;
  s_MousePosition = position;

  // 计算鼠标移动增量
  s_MouseDelta = s_MousePosition - s_PrevMousePosition;
}

void InputManager::SetMouseScrollDelta(float delta)
{
  // 更新鼠标滚轮增量
  s_PrevMouseScrollDelta = s_MouseScrollDelta;
  s_MouseScrollDelta = delta;
}

void InputManager::PushContext(std::shared_ptr<InputContext> context)
{
  if (!context) {
    LOG_ERROR("Attempted to push null input context");
    return;
  }

  // 非空栈情况下，
  // 日志记录Input输入的入栈顺序
  if (!s_ContextStack->IsEmpty()) {
    auto &current = s_ContextStack->GetCurrent();
    LOG_DEBUG("Pushing input context '{}' over '{}'", context->GetName(), current->GetName());
  }
  else {
    LOG_DEBUG("Pushing first input context '{}'", context->GetName());
  }

  // 执行入栈操作
  s_ContextStack->Push(context);
}

void InputManager::PopContext()
{
  if (s_ContextStack->IsEmpty()) {
    LOG_ERROR("Attempted to pop empty input context stack");
    return;
  }

  // 执行出栈操作
  auto popped = s_ContextStack->GetCurrent();
  s_ContextStack->Pop();

  // 出栈后非空栈的情况下，
  // 日志记录下一个Input事件
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
  // 空栈情况下访问当前上下文
  if (s_ContextStack->IsEmpty()) {
    LOG_WARN("No input context available");
    return nullptr;
  }

  return s_ContextStack->GetCurrent();
}

float InputManager::GetActionValue(const std::string &actionName)
{
  // 如果没有活跃的输入上下文，返回0.0
  if (s_ContextStack->IsEmpty())
    return 0.0f;

  // 获取当前上下文
  auto &currentContext = s_ContextStack->GetCurrent();

  // 查找指定名称的动作
  InputAction *action = currentContext->GetAction(actionName);
  if (!action)
    return 0.0f;

  // 重置动作值
  action->value = 0.0f;

  // 遍历动作的所有绑定
  for (const auto &binding : action->bindings) {
    switch (binding.device) {
      case InputDevice::Keyboard: {
        // 如果是键盘绑定且按键被按下，累加缩放值
        if (GetKeyState(binding.code) == InputState::Pressed ||
            GetKeyState(binding.code) == InputState::Held ||
            GetKeyState(binding.code) == InputState::Repeated)
        {
          action->value += binding.scale;
        }
        break;
      }
      case InputDevice::Mouse: {
        // 如果是鼠标按钮绑定且按钮被按下，累加缩放值
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

  // 返回动作的最终值（限制在-1.0到1.0之间）
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
  // 检查鼠标按钮码是否在有效范围内
  if (button >= 0 && button < s_MouseButtonStates.size()) {
    // 返回当前帧的鼠标按钮状态（Pressed 或 Held 都视为按下）
    return s_MouseButtonStates[button];
  }
  return InputState::Released;
}

InputState InputManager::GetPrevMouseButtonState(MouseCode button)
{
  // 检查上一帧鼠标按钮码是否在有效范围内
  if (button >= 0 && button < s_PrevMouseButtonStates.size()) {
    // 返回上一帧的鼠标按钮状态（Pressed 或 Held 都视为按下）
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