#include "command_registry.h"

namespace mite {
CommandRegistry::CommandRegistry()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Registry");
  m_Logger->debug("Command Registry created");
}
// ==================== 单例实现 ====================
CommandRegistry &CommandRegistry::Get()
{
  static CommandRegistry instance;
  return instance;
}

// ==================== 命令创建接口实现 ====================
CommandHandle CommandRegistry::CreateCommand(std::type_index typeIndex)
{
  std::shared_lock readLock(m_typesMutex);

  auto it = m_commandTypes.find(typeIndex);
  if (it == m_commandTypes.end()) {
    m_Logger->warn("Command type not registered: {}", typeIndex.name());
    return CommandHandle();
  }
  readLock.unlock();

  auto command = it->second.creator();
  if (!command) {
    m_Logger->error("Failed to create command instance for type: {}", typeIndex.name());
    return CommandHandle();
  }
  return StoreCommand(std::move(command));
}
CommandPtr CommandRegistry::AcquireCommand(const CommandHandle &handle,
                                           std::type_index expectedType)
{
  if (!handle.IsValid()) {
    return nullptr;
  }
  std::unique_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    m_Logger->warn("Command handle not found: {}", handle.ToString());
    return nullptr;
  }
  // 类型安全检查
  if (expectedType != std::type_index(typeid(void)) && it->second.type != expectedType) {
    m_Logger->error("Command type mismatch. Expected: {}, Actual: {}",
                  expectedType.name(),
                  it->second.type.name());
    return nullptr;
  }

  // 移交所有权
  CommandPtr command = std::move(it->second.command);
  m_commandInstances.erase(it);

  m_Logger->debug("Acquired command: {} ({})", command->GetName(), handle.ToString());
  return command;
}
const Command *CommandRegistry::PeekCommand(const CommandHandle &handle) const
{
  if (!handle.IsValid()) {
    return nullptr;
  }
  std::shared_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);

  // 不移交所有权，仅返回get()
  return it != m_commandInstances.end() ? it->second.command.get() : nullptr;
}
CommandHandle CommandRegistry::StoreCommand(CommandPtr command)
{
  if (!command) {
    return CommandHandle();
  }
  CommandHandle handle = CommandHandle::Create();
  std::type_index typeIndex = typeid(*command);
  std::unique_lock lock(m_instancesMutex);
  m_commandInstances[handle] = {std::move(command), typeIndex, std::chrono::system_clock::now()};
  m_Logger->debug(
      "Stored command: {} ({})", m_commandInstances[handle].command->GetName(), handle.ToString());
  return handle;
}
bool CommandRegistry::IsHandleValid(const CommandHandle &handle) const
{
  if (!handle.IsValid()) {
    return false;
  }
  std::shared_lock lock(m_instancesMutex);
  return m_commandInstances.find(handle) != m_commandInstances.end();
}
bool CommandRegistry::ReleaseCommand(const CommandHandle &handle)
{
  if (!handle.IsValid()) {
    return false;
  }
  std::unique_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    return false;
  }
  m_Logger->debug("Released command: {} ({})", it->second.command->GetName(), handle.ToString());
  m_commandInstances.erase(it);
  return true;
}

// ==================== 批量操作接口 ====================
std::vector<CommandHandle> CommandRegistry::GetActiveHandles() const
{
  std::shared_lock lock(m_instancesMutex);
  std::vector<CommandHandle> handles;
  handles.reserve(m_commandInstances.size());

  for (const auto &pair : m_commandInstances) {
    handles.push_back(pair.first);
  }

  return handles;
}
void CommandRegistry::ClearCommands()
{
  std::unique_lock lock(m_instancesMutex);
  m_Logger->info("Clearing {} command instances", m_commandInstances.size());
  m_commandInstances.clear();
}
size_t CommandRegistry::GetActiveCommandCount() const
{
  std::shared_lock lock(m_instancesMutex);
  return m_commandInstances.size();
}  

// ==================== 类型信息查询接口实现 ====================
std::string CommandRegistry::GetCommandTypeName(std::type_index typeIndex) const
{
  std::shared_lock lock(m_typesMutex);
  auto it = m_commandTypes.find(typeIndex);
  return it != m_commandTypes.end() ? it->second.typeName : nullptr;
}
CommandCategory CommandRegistry::GetCommandCategory(std::type_index typeIndex) const
{
  std::shared_lock lock(m_typesMutex);
  auto it = m_commandTypes.find(typeIndex);
  return it != m_commandTypes.end() ? it->second.category : COMMAND_CATEGORY_NONE;
}
CommandPriority CommandRegistry::GetCommandDefaultPriority(std::type_index typeIndex) const
{
  std::shared_lock lock(m_typesMutex);
  auto it = m_commandTypes.find(typeIndex);
  return it != m_commandTypes.end() ? it->second.defaultPriority : CommandPriority::NORMAL;
}

// ==================== 批量查询接口实现 ====================
CommandRegistry::CommandTypeList CommandRegistry::GetRegisteredCommandTypeIndices() const
{
  std::shared_lock lock(m_typesMutex);
  CommandTypeList indices;
  indices.reserve(m_commandTypes.size());
  for (const auto &pair : m_commandTypes) {
    indices.push_back(pair.first);
  }
  return indices;
}
CommandRegistry::CommandTypeList CommandRegistry::GetCommandTypesByCategory(
    CommandCategory category) const
{
  std::shared_lock lock(m_typesMutex);
  CommandTypeList result;

  for (const auto &pair : m_commandTypes) {
    if ((pair.second.category & category) != 0) {
      result.push_back(pair.first);
    }
  }

  return result;
}
CommandRegistry::CommandTypeList CommandRegistry::GetCommandTypesByPriority(
    CommandPriority priority) const
{
  std::shared_lock lock(m_typesMutex);
  CommandTypeList result;

  for (const auto &pair : m_commandTypes) {
    if (pair.second.defaultPriority == priority) {
      result.push_back(pair.first);
    }
  }

  return result;
}
size_t CommandRegistry::GetCommandTypeCount() const
{
  std::shared_lock lock(m_typesMutex);
  return m_commandTypes.size();
}

// ==================== 注册表管理接口实现 ====================
void CommandRegistry::Clear()
{
  std::shared_lock lock(m_typesMutex);
  m_commandTypes.clear();
}
bool CommandRegistry::IsEmpty() const
{
  std::shared_lock lock(m_typesMutex);
  return m_commandTypes.empty();
}
// ==================== 类型检查接口实现 ====================
bool CommandRegistry::IsCommandTypeRegistered(std::type_index typeIndex) const
{
  std::shared_lock lock(m_typesMutex);
  return m_commandTypes.find(typeIndex) != m_commandTypes.end();
}

}  // namespace mite
