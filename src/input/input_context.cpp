#include "input_context.h"

namespace mite {
InputContext::InputContext(const std::string &name) : m_Name(name)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Context");
  m_Logger->trace("Created input context: {}", name);
}

InputContext::~InputContext()
{
  m_Logger->trace("Created input context: {}", m_Name);
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

bool InputContext::ProcessEvent(Event &event)
{
  // 1. ���ȫ������
  if (m_BlockInput)
    return true;

  // 2. ����ӳ��ϵͳ����
  EventDispatcher dispatcher(event);

  dispatcher.Dispatch<KeyEvent>([this](KeyEvent &e) { return _ProcessKeyEvent(e); });

  dispatcher.Dispatch<MouseButtonEvent>(
      [this](MouseButtonEvent &e) { return _ProcessMouseButtonEvent(e); });
  dispatcher.Dispatch<MouseMoveEvent>(
      [this](MouseMoveEvent &e) { return _ProcessMouseMoveEvent(e); });

  // 2. �����Ƿ������¼�
  return event.handled;
}

void InputContext::Update(){
    // TODO: ÿ֡���³�������ʱ�䣨�糤����
  for (auto &[name, action] : m_Actions) {
    if (action.value > 0.0f) {
      action.hold_time += Time::DeltaTime();
    }
    else {
      action.hold_time = 0.0f;
    }
  }
}

void InputContext::DebugPrintActions() const
{
  m_Logger->debug("=== Actions in {} ===", m_Name);
  for (const auto &[name, action] : m_Actions) {
    m_Logger->debug("{}: value={}, hold={}s", name, action.value, action.hold_time);
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

bool InputContext::_ProcessKeyEvent(const KeyEvent &e)
{
  bool consumed = false;

  // �������ж���������Ƿ�ƥ�䵱ǰ����
  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Keyboard && binding.code == e.GetKey()) {
        // �����¼����͸��¶���ֵ
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

bool InputContext::_ProcessMouseButtonEvent(const MouseButtonEvent &e)
{
  bool consumed = false;

  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse && binding.code == e.GetButton()) {
        float newValue = 0.0f;
        if (e.GetEventType() == EventType::MouseButtonPressed)
          newValue = 1.0f * binding.scale;
        else if (e.GetEventType() == EventType::MouseButtonReleased)
          newValue = 0.0f;

        _UpdateActionValue(name, newValue);
        consumed = true;
      }
    }
  }

  return consumed;
}

bool InputContext::_ProcessMouseMoveEvent(const MouseMoveEvent &e)
{
  bool consumed = false;

  for (auto &[name, action] : m_Actions) {
    for (const auto &binding : action.bindings) {
      if (binding.device == InputDevice::Mouse) {
          // TODO: ����ƶ���Ϊ����Ƶ�¼����Ƿ�Ӧ���������ﴦ��
        float newValue = (e.GetEventType() == EventType::MouseMoved) ? 1.0f * binding.scale : 0.0f;

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
  }
}

};  // namespace mite