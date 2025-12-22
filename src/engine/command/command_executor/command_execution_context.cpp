#include "command_execution_context.h"

#include "command_core/command_event.h"

namespace mite {
// ==================== 构造函数和析构函数实现 ====================

CommandExecutionContext::CommandExecutionContext(
    CommandContextFlags contextFlags, const std::string &name)
    : m_contextFlags(contextFlags),
      m_name(name),
      m_isActive(false),
      m_ExecutingCount(0),
      m_CompletedCount(0),
      m_SucceededCount(0),
      m_FailedCount(0) {
  m_Logger = mite::LoggerSystem::CreateModuleLogger(
      "Mite Command Executor Context" + name);
  m_Logger->debug("CommandExecutionContext '{}' created with flags: {}", name,
                  static_cast<int>(contextFlags));
}

CommandExecutionContext::~CommandExecutionContext() {
  std::unique_lock lock(m_ExecutionMutex);
  if (!m_ExecutingCommands.empty()) {
    m_Logger->warn(
        "CommandExecutionContext '{}' destroyed with {} executing commands",
        m_name, m_ExecutingCommands.size());
  }
}

// ==================== 上下文标志管理接口实现 ====================
CommandContextFlags CommandExecutionContext::GetContextFlags() const {
  return m_contextFlags;
}
void CommandExecutionContext::SetContextFlags(CommandContextFlags flags) {
  m_contextFlags = flags;
}
void CommandExecutionContext::AddContextFlags(CommandContextFlags flags) {
  uint32_t oleFlags = static_cast<uint32_t>(m_contextFlags);
  m_contextFlags = static_cast<CommandContextFlags>(
      oleFlags |= static_cast<uint32_t>(flags));
}
void CommandExecutionContext::RemoveContextFlags(CommandContextFlags flags) {
  uint32_t oleFlags = static_cast<uint32_t>(m_contextFlags);
  m_contextFlags = static_cast<CommandContextFlags>(
      oleFlags &= ~static_cast<uint32_t>(flags));
}
bool CommandExecutionContext::HasContextFlags(CommandContextFlags flags) const {
  return (m_contextFlags & flags) != 0;
}

// ==================== 上下文状态管理接口实现 ====================
bool CommandExecutionContext::IsActive() const { return m_isActive; }
void CommandExecutionContext::Activate() {
  if (!m_isActive) {
    m_isActive = true;
    m_Logger->debug("CommandExecutionContext '{}' activated", m_name);
  }
}
void CommandExecutionContext::Deactivate() {
  if (m_isActive) {
    m_isActive = false;
    m_Logger->debug("CommandExecutionContext '{}' deactivated", m_name);
  }
}
const std::string &CommandExecutionContext::GetName() const { return m_name; }

// ==================== 命令可用性检查接口实现 ====================
bool CommandExecutionContext::IsCommandAvailable(
    const CommandRegistry &registry, const CommandHandle &handle) const {
  if (!handle.IsValid()) {
    m_Logger->warn("Invalid command handle passed to IsCommandAvailable");
    return false;
  }
  // 基础检查：上下文是否活动
  if (!m_isActive) {
    m_Logger->debug("Context '{}' is not active", m_name);
    return false;
  }
  // 获取命令对象进行检查（不转移所有权）
  const Command *command = registry.PeekCommand(handle);
  if (!command) {
    m_Logger->warn("Command handle not found: {}", handle.ToString());
    return false;
  }
  // 检查命令状态是否为可执行状态
  if (!registry.IsCommandExecutable(handle)) {
    m_Logger->debug("Command '{}' is not in executable state",
                    command->GetName());
    return false;
  }
  m_Logger->trace("Command '{}' is available in context '{}'",
                  command->GetName(), m_name);
  return true;
}
bool CommandExecutionContext::IsCommandTypeAvailable(
    const CommandRegistry &registry, std::type_index typeIndex) const {
  if (!m_isActive) {
    return false;
  }
  // 检查注册情况
  if (!registry.IsCommandTypeRegistered(typeIndex)) {
    m_Logger->debug("Command type '{}' is not registered", typeIndex.name());
    return false;
  }

  return true;
}

// ==================== 命令执行跟踪接口实现 ====================
void CommandExecutionContext::RecordCommandExecutionStart(
    const CommandHandle &handle) {
  if (!handle.IsValid()) return;
  std::unique_lock lock(m_ExecutionMutex);

  // 更新统计信息
  if (m_ExecutingCommands.insert(handle).second) {
    m_ExecutingCount++;
    m_Logger->trace("Command handle {} started execution in context '{}'",
                    handle.ToString(), m_name);
  }
}
void CommandExecutionContext::RecordCommandExecutionComplete(
    const CommandHandle &handle, const CommandResult &result) {
  if (!handle.IsValid()) return;
  std::unique_lock lock(m_ExecutionMutex);

  // 更新统计信息
  if (m_ExecutingCommands.erase(handle) > 0) {
    m_ExecutingCount--;
    m_CompletedCount++;
    if (result.success) {
      m_SucceededCount++;
    } else {
      m_FailedCount++;
    }
    m_Logger->trace(
        "Command handle {} completed in context '{}' with result: {}",
        handle.ToString(), m_name, result.success ? "success" : "failure");
  }
}
size_t CommandExecutionContext::GetExecutingCommandCount() const {
  std::shared_lock lock(m_ExecutionMutex);
  return m_ExecutingCount;
}
size_t CommandExecutionContext::GetCompletedCommandCount() const {
  std::shared_lock lock(m_ExecutionMutex);
  return m_CompletedCount;
}
size_t CommandExecutionContext::GetSucceededCommandCount() const {
  std::shared_lock lock(m_ExecutionMutex);
  return m_SucceededCount;
}
size_t CommandExecutionContext::GetFailedCommandCount() const {
  std::shared_lock lock(m_ExecutionMutex);
  return m_FailedCount;
}

// ==================== 句柄管理接口实现 ====================
std::vector<CommandHandle> CommandExecutionContext::GetExecutingCommandHandles()
    const {
  std::shared_lock lock(m_ExecutionMutex);
  return std::vector<CommandHandle>(m_ExecutingCommands.begin(),
                                    m_ExecutingCommands.end());
}
bool CommandExecutionContext::IsCommandExecuting(
    const CommandHandle &handle) const {
  std::shared_lock lock(m_ExecutionMutex);
  return m_ExecutingCommands.find(handle) != m_ExecutingCommands.end();
}
void CommandExecutionContext::ClearExecutionRecords() {
  std::unique_lock lock(m_ExecutionMutex);
  m_ExecutingCommands.clear();
  m_ExecutingCount = 0;
  m_CompletedCount = 0;
  m_SucceededCount = 0;
  m_FailedCount = 0;
}
}  // namespace mite