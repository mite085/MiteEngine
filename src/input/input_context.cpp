#include "input_context.h"

namespace mite {
InputContext::InputContext(const std::string &name) : m_Name(name)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Context");
  m_Logger->debug("Created input context: {}", name);
}

const std::string &InputContext::GetName() const
{
  return m_Name;
}

bool InputContext::ProcessEvent(Event &event)
{
  // 1. 根据事件类型分发处理
  bool handled = false;
  EventDispatcher dispatcher(event);

  dispatcher.Dispatch<KeyEvent>([this](KeyEvent &e) { return _ProcessKeyEvent(e); });

  dispatcher.Dispatch<MouseButtonEvent>(
      [this](MouseButtonEvent &e) { return _ProcessMouseEvent(e); });

  // 2. 返回是否消费事件
  return handled;
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

bool InputContext::_ProcessKeyEvent(const KeyEvent &e)
{
  bool consumed = false;

  // 遍历所有动作，检查是否匹配当前按键
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Keyboard && binding.code == e.GetKey()) {

        // 根据事件类型更新动作值
        float newValue = 0.0f;
        if (e.GetEventType() == EventType::KeyPressed) {
          newValue = 1.0f * binding.scale;
        }
        else if (e.GetEventType() == EventType::KeyReleased) {
          newValue = 0.0f;
        }

        _UpdateActionValue(name, newValue);
        consumed = true;
      }
    }
  }

  return consumed;
}

bool InputContext::_ProcessMouseEvent(const MouseButtonEvent &e)
{
  bool consumed = false;

  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse && binding.code == e.GetButton()) {

        float newValue = (e.GetEventType() == EventType::MouseButtonPressed) ?
                             1.0f * binding.scale :
                             0.0f;

        _UpdateActionValue(name, newValue);
        consumed = true;
      }
    }
  }

  return consumed;
}

void InputContext::_UpdateActionValue(const std::string &actionName, float newValue)
{
  auto it = m_Actions.find(actionName);
  if (it != m_Actions.end()) {
    it->second.value = newValue;

    // TODO：可以在这里触发回调或发送动作事件
    // OnActionChanged(actionName, newValue);
  }
}

void InputContext::BlockInput(bool block)
{
  m_BlockInput = block;
}

bool InputContext::IsInputBlocked() const
{
  return m_BlockInput;
}
InputAction::Binding::Binding(InputDevice device, int code, float scale)
    : device(device), code(code), scale(scale)
{
}

InputContextStack::InputContextStack() {
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Context Stack");
  m_Logger->debug("Created input context stack");
}

void InputContextStack::Push(const std::shared_ptr<InputContext> &context)
{
  if (!context) {
    m_Logger->error("Attempted to push null InputContext!");
    return;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // 防止重复推入相同实例
  if (!m_Stack.empty() && m_Stack.back() == context) {
    m_Logger->warn("Duplicate context pushed: {}", context->GetName());
    return;
  }

  m_Stack.push_back(context);
  m_Logger->trace("Context pushed: {}", context->GetName());
}

void InputContextStack::Pop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Stack.empty()) {
    m_Logger->error("Attempted to pop empty InputContextStack!");
    return;
  }

  m_Logger->trace("Context popped: {}", m_Stack.back()->GetName());
  m_Stack.pop_back();
}

std::shared_ptr<InputContext> InputContextStack::GetCurrent() 
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Stack.empty()) {
    return nullptr;
  }
  return m_Stack.back();
}

bool InputContextStack::IsInContext(const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 从栈顶向下搜索
  auto it = std::find_if(
      m_Stack.rbegin(), m_Stack.rend(), [&name](const std::shared_ptr<InputContext> &ctx) {
        return ctx->GetName() == name;
      });

  return it != m_Stack.rend();
}

bool InputContextStack::ProcessEvent(Event &event)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 从栈顶向下处理
  for (auto it = m_Stack.rbegin(); it != m_Stack.rend(); ++it) {
    const auto &context = *it;

    // 1. 检查是否阻塞输入
    if (context->IsInputBlocked()) {
      m_Logger->trace("Event blocked by context: {}", context->GetName());
      return true;
    }

    // 2. 尝试处理事件
    if (context->ProcessEvent(event)) {
      m_Logger->trace("Event consumed by context: {}", context->GetName());
      return true;
    }
  }

  return false;
}

bool InputContextStack::IsEmpty()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stack.empty();
}

// 辅助方法：清空栈（用于场景切换等）
void InputContextStack::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Logger->info("Clearing all {} input contexts", m_Stack.size());
  m_Stack.clear();
}
};  // namespace mite