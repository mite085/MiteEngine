#include "command_execution_context.h"
#include "command_core/command_event.h"
#include "command_core/command_registry.h"

namespace mite {

// ==================== 构造函数和析构函数实现 ====================

CommandExecutionContext::CommandExecutionContext(uint32_t contextFlags, const std::string &name)
    : m_contextFlags(contextFlags),
      m_name(name),
      m_isActive(false),
      m_executingCount(0),
      m_completedCount(0),
      m_succeededCount(0),
      m_failedCount(0)
{
  spdlog::debug("CommandExecutionContext '{}' created with flags: {}", name, contextFlags);
}

CommandExecutionContext::~CommandExecutionContext()
{
  if (!m_ExecutingCommands.empty()) {
    spdlog::warn("CommandExecutionContext '{}' destroyed with {} executing commands",
                 m_name,
                 m_ExecutingCommands.size());
  }
}

// ==================== 上下文标志管理接口实现 ====================

uint32_t CommandExecutionContext::GetContextFlags() const
{
  return m_contextFlags;
}

void CommandExecutionContext::SetContextFlags(uint32_t flags)
{
  m_contextFlags = flags;
}

void CommandExecutionContext::AddContextFlags(uint32_t flags)
{
  m_contextFlags |= flags;
}

void CommandExecutionContext::RemoveContextFlags(uint32_t flags)
{
  m_contextFlags &= ~flags;
}

bool CommandExecutionContext::HasContextFlags(uint32_t flags) const
{
  return (m_contextFlags & flags) != 0;
}

// ==================== 上下文状态管理接口实现 ====================

bool CommandExecutionContext::IsActive() const
{
  return m_isActive;
}

void CommandExecutionContext::Activate()
{
  if (!m_isActive) {
    m_isActive = true;
    spdlog::debug("CommandExecutionContext '{}' activated", m_name);
  }
}

void CommandExecutionContext::Deactivate()
{
  if (m_isActive) {
    m_isActive = false;
    spdlog::debug("CommandExecutionContext '{}' deactivated", m_name);
  }
}

const std::string &CommandExecutionContext::GetName() const
{
  return m_name;
}

// ==================== 命令可用性检查接口实现 ====================
bool CommandExecutionContext::IsCommandAvailable(Command *command) const
{
  if (!command) {
    spdlog::warn("Null command passed to IsCommandAvailable");
    return false;
  }

  // 基础检查：上下文是否活动
  if (!m_isActive) {
    spdlog::debug("Context '{}' is not active", m_name);
    return false;
  }

  // 检查命令状态
  if (!command->IsExecutable()) {
    spdlog::debug("Command '{}' is not in executable state", command->GetName());
    return false;
  }

  // 检查类别限制
  if (!CheckCommandCategory(command)) {
    spdlog::debug("Command '{}' category not allowed in context '{}'", command->GetName(), m_name);
    return false;
  }

  // 检查优先级限制
  if (!CheckCommandPriority(command)) {
    spdlog::debug("Command '{}' priority not allowed in context '{}'", command->GetName(), m_name);
    return false;
  }

  // 检查特定类型禁止列表
  std::type_index typeIndex = typeid(*command);
  if (m_forbiddenCommandTypes.find(typeIndex) != m_forbiddenCommandTypes.end()) {
    spdlog::debug("Command type '{}' is forbidden in context '{}'", typeIndex.name(), m_name);
    return false;
  }

  // 检查特定类型允许列表（如果设置了允许列表）
  if (!m_allowedCommandTypes.empty() &&
      m_allowedCommandTypes.find(typeIndex) == m_allowedCommandTypes.end())
  {
    spdlog::debug(
        "Command type '{}' is not in allowed list for context '{}'", typeIndex.name(), m_name);
    return false;
  }

  // 应用自定义过滤器
  for (const auto &filter : m_commandFilters) {
    if (!filter(command)) {
      spdlog::debug(
          "Command '{}' rejected by custom filter in context '{}'", command->GetName(), m_name);
      return false;
    }
  }

  // 上下文特定的检查（子类可以重写）
  if (m_contextFlags != CONTEXT_NONE) {
    // 示例：编辑器上下文只允许编辑器相关的命令
    if ((m_contextFlags & CONTEXT_EDITOR) && !command->IsInCategory(COMMAND_CATEGORY_EDITOR)) {
      spdlog::debug("Non-editor command '{}' not allowed in editor context", command->GetName());
      return false;
    }

    // 示例：运行时上下文禁止编辑器命令
    if ((m_contextFlags & CONTEXT_RUNTIME) && command->IsInCategory(COMMAND_CATEGORY_EDITOR)) {
      spdlog::debug("Editor command '{}' not allowed in runtime context", command->GetName());
      return false;
    }
  }

  spdlog::trace("Command '{}' is available in context '{}'", command->GetName(), m_name);
  return true;
}
bool CommandExecutionContext::IsCommandTypeAvailable(std::type_index typeIndex) const
{
  // 基础检查：上下文是否活动
  if (!m_isActive) {
    return false;
  }

  // 检查类型是否在注册表中
  if (!CommandRegistry::Get().IsCommandTypeRegistered(typeIndex)) {
    spdlog::debug("Command type '{}' is not registered", typeIndex.name());
    return false;
  }

  // 检查特定类型禁止列表
  if (m_forbiddenCommandTypes.find(typeIndex) != m_forbiddenCommandTypes.end()) {
    return false;
  }

  // 检查特定类型允许列表（如果设置了允许列表）
  if (!m_allowedCommandTypes.empty() &&
      m_allowedCommandTypes.find(typeIndex) == m_allowedCommandTypes.end())
  {
    return false;
  }

  // 应用自定义类型过滤器
  for (const auto &filter : m_typeFilters) {
    if (!filter(typeIndex)) {
      return false;
    }
  }

  // 获取命令的默认信息（通过创建临时实例）
  if (auto tempCommand = CommandRegistry::Get().CreateCommand(typeIndex)) {
    // 检查类别限制
    CommandCategory category = tempCommand->GetCategory();
    if ((category & m_forbiddenCategories) != 0) {
      return false;
    }
    if ((category & m_allowedCategories) != category) {
      return false;
    }

    // 检查优先级限制
    CommandPriority priority = tempCommand->GetPriority();
    if (priority < m_minPriority || priority > m_maxPriority) {
      return false;
    }
  }

  return true;
}

// ==================== 过滤器管理接口实现 ====================
void CommandExecutionContext::AddCommandFilter(CommandFilter filter)
{
  m_commandFilters.push_back(std::move(filter));
}
void CommandExecutionContext::AddTypeFilter(TypeFilter filter)
{
  m_typeFilters.push_back(std::move(filter));
}
void CommandExecutionContext::ClearFilters()
{
  m_commandFilters.clear();
  m_typeFilters.clear();
}
void CommandExecutionContext::SetAllowedCategories(CommandCategory allowedCategories)
{
  m_allowedCategories = allowedCategories;
}
void CommandExecutionContext::SetForbiddenCategories(CommandCategory forbiddenCategories)
{
  m_forbiddenCategories = forbiddenCategories;
}
void CommandExecutionContext::AddAllowedCommandType(std::type_index typeIndex)
{
  m_allowedCommandTypes.insert(typeIndex);
}
void CommandExecutionContext::AddForbiddenCommandType(std::type_index typeIndex)
{
  m_forbiddenCommandTypes.insert(typeIndex);
}
void CommandExecutionContext::SetMinCommandPriority(CommandPriority minPriority)
{
  m_minPriority = minPriority;
}
void CommandExecutionContext::SetMaxCommandPriority(CommandPriority maxPriority)
{
  m_maxPriority = maxPriority;
}

// ==================== 命令执行跟踪接口实现 ====================

void CommandExecutionContext::RecordCommandExecutionStart(Command *command)
{
  if (command && m_ExecutingCommands.insert(command).second) {
    m_executingCount++;
    spdlog::trace("Command '{}' started execution in context '{}'", command->GetName(), m_name);
  }
}

void CommandExecutionContext::RecordCommandExecutionComplete(Command *command,
                                                             const CommandResult &result)
{
  if (command && m_ExecutingCommands.erase(command) > 0) {
    m_executingCount--;
    m_completedCount++;

    if (result.success) {
      m_succeededCount++;
    }
    else {
      m_failedCount++;
    }

    spdlog::trace("Command '{}' completed in context '{}' with result: {}",
                  command->GetName(),
                  m_name,
                  result.success ? "success" : "failure");
  }
}

size_t CommandExecutionContext::GetExecutingCommandCount() const
{
  return m_executingCount;
}

size_t CommandExecutionContext::GetCompletedCommandCount() const
{
  return m_completedCount;
}

size_t CommandExecutionContext::GetSucceededCommandCount() const
{
  return m_succeededCount;
}

size_t CommandExecutionContext::GetFailedCommandCount() const
{
  return m_failedCount;
}

// ==================== 验证工具接口实现 ====================
std::vector<std::type_index> CommandExecutionContext::GetAllowedCommandTypes() const
{
  std::vector<std::type_index> result;

  if (m_allowedCommandTypes.empty()) {
    // 如果没有设置特定允许类型，返回所有注册的类型中可用的
    auto allTypes = CommandRegistry::Get().GetRegisteredCommandTypeIndices();
    for (const auto &typeIndex : allTypes) {
      if (IsCommandTypeAvailable(typeIndex)) {
        result.push_back(typeIndex);
      }
    }
  }
  else {
    // 只返回允许列表中可用的类型
    for (const auto &typeIndex : m_allowedCommandTypes) {
      if (IsCommandTypeAvailable(typeIndex)) {
        result.push_back(typeIndex);
      }
    }
  }

  return result;
}
bool CommandExecutionContext::CheckCommandPriority(const Command *command) const
{
  CommandPriority priority = command->GetPriority();
  return priority >= m_minPriority && priority <= m_maxPriority;
}
bool CommandExecutionContext::CheckCommandCategory(const Command *command) const
{
  CommandCategory category = command->GetCategory();

  // 检查禁止类别
  if ((category & m_forbiddenCategories) != 0) {
    return false;
  }

  // 检查允许类别
  if ((category & m_allowedCategories) != category) {
    return false;
  }

  return true;
}

}  // namespace mite
