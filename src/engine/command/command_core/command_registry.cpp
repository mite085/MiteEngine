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

  // 检查命令是否已被获取
  if (it->second.isAcquired) {
    m_Logger->warn("Command handle {} already acquired", handle.ToString());
    return nullptr;
  }
  // 标记为正在执行
  it->second.state = CommandExecutionState::EXECUTING;
  it->second.isAcquired = true;

  // 移交所有权
  CommandPtr command = std::move(it->second.command);

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
  std::unique_lock lock(m_instancesMutex);

  CommandHandle handle = CommandHandle::Create();
  std::type_index typeIndex = typeid(*command);

  // 使用新的句柄创建新的实例
  m_commandInstances.try_emplace(handle,
                                 std::move(command),
                                 typeIndex,
                                 CommandExecutionState::PENDING  // 新实例直接设定为Pending
  );

  m_Logger->debug("Stored command, handle: {}", handle.ToString());
  return handle;
}
bool CommandRegistry::ReStoreCommand(const CommandHandle &handle,
                                     CommandPtr command)
{
  if (!handle.IsValid() || !command) {
    return false;
  }
  std::unique_lock lock(m_instancesMutex);

  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    // 如果句柄不存在，创建新的实例
    std::type_index typeIndex = typeid(*command);
    m_commandInstances.try_emplace(
        handle,
        std::move(command),
        typeIndex,
        CommandExecutionState::PENDING  // 新实例无视state输入，直接设定为Pending，确保状态管理正确
    );
  }
  else {
    // 替换现有实例
    if (it->second.command && !it->second.isAcquired) {
      m_Logger->warn("Handle {} already has associated command", handle.ToString());
      return false;
    }

    it->second.command = std::move(command);
    it->second.type = typeid(*it->second.command);
    it->second.createTime = std::chrono::system_clock::now();
    //it->second.state = state;                         // 保持原有状态
    it->second.isAcquired = false;                      // 重置获取
  }
  m_Logger->debug("Re-stored command to handle: {}", handle.ToString());
  return true;
}
bool CommandRegistry::HasCommand(const CommandHandle &handle) const
{
  if (!handle.IsValid()) {
    return false;
  }
  std::shared_lock lock(m_instancesMutex);
  return m_commandInstances.find(handle) != m_commandInstances.end();
}
CommandHandle CommandRegistry::PreAllocateHandle()
{
  CommandHandle handle = CommandHandle::Create();

  std::unique_lock lock(m_instancesMutex);
  // 预分配时命令对象为空
  m_commandInstances.try_emplace(handle, nullptr, std::type_index(typeid(void)));

  m_Logger->debug("Pre-allocated handle: {}", handle.ToString());
  return handle;
}
bool CommandRegistry::AssociateCommand(const CommandHandle &handle, CommandPtr command)
{
  if (!handle.IsValid() || !command) {
    return false;
  }
  std::unique_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    m_Logger->warn("Handle {} not found for association", handle.ToString());
    return false;
  }
  if (it->second.command) {
    m_Logger->warn("Handle {} already has associated command", handle.ToString());
    return false;
  }
  it->second.command = std::move(command);
  it->second.type = typeid(*it->second.command);
  it->second.createTime = std::chrono::system_clock::now();
  m_Logger->debug("Associated command to pre-allocated handle: {}", handle.ToString());
  return true;
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
// ==================== 状态管理接口实现 ====================
bool CommandRegistry::SetCommandState(const CommandHandle &handle, CommandExecutionState state)
{
  if (!handle.IsValid()) {
    return false;
  }
  std::unique_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    return false;
  }
  // 状态转换验证
  if (!ValidateStateTransition(it->second.state, state)) {
    m_Logger->warn("Invalid state transition from {} to {} for handle {}",
                   CommandResult::StateToString(it->second.state),
                   CommandResult::StateToString(state),
                   handle.ToString());
    return false;
  }
  it->second.state = state;
  m_Logger->trace("Command handle {} state changed to {}",
                  handle.ToString(),
                  CommandResult::StateToString(state));
  return true;
}
CommandExecutionState CommandRegistry::GetCommandState(const CommandHandle &handle) const
{
  if (!handle.IsValid()) {
    return CommandExecutionState::INVALID; // 句柄不存在视为Invalid
  }
  std::shared_lock lock(m_instancesMutex);
  auto it = m_commandInstances.find(handle);
  if (it == m_commandInstances.end()) {
    return CommandExecutionState::INVALID;  // 句柄未被管理也视为Invalid
  }
  return it->second.state;
}
bool CommandRegistry::IsCommandExecutable(const CommandHandle &handle) const
{
  CommandExecutionState state = GetCommandState(handle);

  // 待执行、待撤销（Successed）、待重做（UnDone）、待再次撤销（ReDone）完成均视为Executable
  return state == CommandExecutionState::PENDING || state == CommandExecutionState::SUCCEEDED ||
         state == CommandExecutionState::UNDONE || state == CommandExecutionState::REDONE;
}
bool CommandRegistry::IsCommandExecuting(const CommandHandle &handle) const
{
  CommandExecutionState state = GetCommandState(handle);

  // 正在执行、正在撤销、正在重做均是为Executing
  return state == CommandExecutionState::EXECUTING || state == CommandExecutionState::UNDOING ||
         state == CommandExecutionState::REDOING;
}
bool CommandRegistry::IsCommandCompleted(const CommandHandle &handle) const
{
  CommandExecutionState state = GetCommandState(handle);

  // 成功、失败、撤销完成、重做完成均视为Complete
  return state == CommandExecutionState::SUCCEEDED || state == CommandExecutionState::FAILED ||
         state == CommandExecutionState::UNDONE || state == CommandExecutionState::REDONE;
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
// ==================== 内部辅助方法 ====================
bool CommandRegistry::ValidateStateTransition(CommandExecutionState from, CommandExecutionState to)
{
  // 定义允许的状态转换
  static const std::unordered_map<CommandExecutionState, std::unordered_set<CommandExecutionState>>
      validTransitions = {{CommandExecutionState::PENDING,
                           {CommandExecutionState::EXECUTING}},  // 待执行状态仅可以转换为执行状态

                          {CommandExecutionState::EXECUTING,
                           {CommandExecutionState::SUCCEEDED,
                            CommandExecutionState::FAILED}},  // 执行状态仅有成功和失败两个结果

                          {CommandExecutionState::SUCCEEDED,
                           {CommandExecutionState::UNDOING}},  // 执行成功后才可转撤销状态

                          {CommandExecutionState::FAILED,
                           {CommandExecutionState::PENDING}},  // 失败后可以转待执行状态等待重试

                          {CommandExecutionState::UNDOING,
                           {CommandExecutionState::UNDONE,
                            CommandExecutionState::FAILED}},  // 撤销状态仅有成功和失败两个结果

                          {CommandExecutionState::UNDONE,
                           {CommandExecutionState::REDOING}},  // 撤销成功后可以转重做

                          {CommandExecutionState::REDOING,
                           {CommandExecutionState::REDONE,
                            CommandExecutionState::FAILED}},  // 撤销状态仅有成功和失败两个结果

                          {CommandExecutionState::REDONE,
                           {CommandExecutionState::UNDOING}}};  // 重做成功后可以转撤销
  auto it = validTransitions.find(from);
  if (it == validTransitions.end()) {
    return false;
  }
  return it->second.find(to) != it->second.end();
}
}  // namespace mite