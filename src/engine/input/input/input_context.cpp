#include "input_context.h"

namespace mite {
InputContext::InputContext(const std::string &name) : m_Name(name)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Context: {" + name + "}");
  m_Logger->trace("Created input context: {}", name);

  // 订阅事件
  // Immediate同步模式：
  // 输入事件需要立即处理以确保游戏响应的实时性，避免输入延迟影响体验
  m_EventSubscriptions.SubscribeImmediate<KeyPressedEvent>(
      BIND_DISPATCH_FN(_ProcessKeyPressedEvent),
      EventPriority::Highest  // 输入事件优先级最高
  );
  m_EventSubscriptions.SubscribeImmediate<MouseButtonPressedEvent>(
      BIND_DISPATCH_FN(_ProcessMouseButtonPressedEvent), EventPriority::Highest);
  m_EventSubscriptions.SubscribeImmediate<MouseMoveEvent>(
      BIND_DISPATCH_FN(_ProcessMouseMoveEvent),
      EventPriority::High  // 鼠标移动频率高，但优先级仍较高
  );
  m_EventSubscriptions.SubscribeImmediate<MouseScrollEvent>(
      BIND_DISPATCH_FN(_ProcessMouseScrollEvent), EventPriority::Highest);
}

InputContext::~InputContext()
{
  m_Logger->trace("Destroy input context: {}", m_Name);
  m_EventSubscriptions.UnsubscribeAll();
}

const std::string &InputContext::GetName() const
{
  return m_Name;
}

void InputContext::SetBlockInput(bool block)
{
  m_BlockInput = block;
}

bool InputContext::IsInputBlocked() const
{
  return m_BlockInput;
}

void InputContext::Update(){
   // 每帧更新持续动作时间（如长按）
  for (auto &[name, action] : m_Actions) {
    if (action.value > 0.0f) {
      action.holdTime += Time::DeltaTime();
    }
    else {
      action.holdTime = 0.0f;
    }
  }
}

void InputContext::DebugPrintActions() const
{
  m_Logger->debug("=== Actions in {} ===", m_Name);
  for (const auto &[name, action] : m_Actions) {
    m_Logger->debug("{}: value={}, hold={}s", name, action.value, action.holdTime);
  }
}

void InputContext::AddAction(const InputAction &action)
{
  if (m_Actions.find(action.name) != m_Actions.end()) {
    m_Logger->warn("Input action '{}' already exists in context '{}', it will be overwritten",
                   action.name,
                   m_Name);
  }

  m_Actions[action.name] = action;
}

void InputContext::RemoveAction(const std::string &actionName)
{
  auto it = m_Actions.find(actionName);
  if (it != m_Actions.end()) {
    m_Actions.erase(it);
  }
  else {
    m_Logger->warn("Attempted to remove non-existent input action '{}' from context '{}'",
                   actionName,
                   m_Name);
  }
}

InputAction *InputContext::GetAction(const std::string &actionName)
{
  auto it = m_Actions.find(actionName);
  if (it != m_Actions.end()) {
    return &it->second;
  }

  m_Logger->warn("Requested non-existent input action '{}' from context '{}'", actionName, m_Name);
  return nullptr;
}

float InputContext::GetActionValue(const std::string &name) const
{
  auto it = m_Actions.find(name);
  return it != m_Actions.end() ? it->second.value : 0.0f;
}

void InputContext::_ProcessKeyPressedEvent(KeyPressedEvent &e)
{
  bool handled = false;
  // 遍历所有动作，检查是否匹配当前按键
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Keyboard && binding.code == e.GetKey()) {
        // 更新动作值
        float newValue = 1.0f * binding.scale;

        _UpdateActionValue(name, newValue);
        handled = true;
      }
    }
  }
  e.SetResult(handled ? EventResult::Handled : EventResult::None);
}

void InputContext::_ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e)
{
  bool handled = false;
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse && binding.code == e.GetButton()) {
        float newValue = 1.0f * binding.scale;

        _UpdateActionValue(name, newValue);
        handled = true;
      }
    }
  }
  e.SetResult(handled ? EventResult::Handled : EventResult::None);
}

void InputContext::_ProcessMouseMoveEvent(MouseMoveEvent &e)
{
  bool handled = false;
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse) {
        // 鼠标移动作为超高频事件，可以优化处理频率
        float newValue = 1.0f * binding.scale;

        _UpdateActionValue(name, newValue);
        handled = true;
      }
    }
  }
  e.SetResult(handled ? EventResult::Handled : EventResult::None);
}

void InputContext::_ProcessMouseScrollEvent(MouseScrollEvent &e)
{
  bool handled = false;
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse) {
        // 鼠标滚轮事件通常需要立即处理
        float newValue = 1.0f * binding.scale;

        _UpdateActionValue(name, newValue);
        handled = true;
      }
    }
  }
  e.SetResult(handled ? EventResult::Handled : EventResult::None);
}

void InputContext::_UpdateActionValue(const std::string &actionName, float newValue)
{
  auto it = m_Actions.find(actionName);
  if (it != m_Actions.end()) {
    it->second.value = newValue;
  }
}

};  // namespace mite