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
};  // namespace mite