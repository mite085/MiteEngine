#include "command_registry.h"

namespace mite {

CommandRegistry &CommandRegistry::Get()
{
  static CommandRegistry instance;
  return instance;
}

CommandPtr CommandRegistry::CreateCommand(std::type_index typeIndex) const
{
  auto it = m_commandTypes.find(typeIndex);
  if (it != m_commandTypes.end()) {
    return it->second.creator();
  }
  return nullptr;
}

bool CommandRegistry::IsCommandTypeRegistered(std::type_index typeIndex) const
{
  return m_commandTypes.find(typeIndex) != m_commandTypes.end();
}

const char *CommandRegistry::GetCommandTypeName(std::type_index typeIndex) const
{
  auto it = m_commandTypes.find(typeIndex);
  return it != m_commandTypes.end() ? it->second.typeName : nullptr;
}

std::vector<std::type_index> CommandRegistry::GetRegisteredCommandTypeIndices() const
{
  std::vector<std::type_index> indices;
  indices.reserve(m_commandTypes.size());

  for (const auto &pair : m_commandTypes) {
    indices.push_back(pair.first);
  }

  return indices;
}

void CommandRegistry::Clear()
{
  m_commandTypes.clear();
}

}  // namespace mite
