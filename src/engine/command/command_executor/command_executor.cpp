#include "command_executor.h"
#include "command_core/command_event.h"


namespace mite {
// ==================== 构造函数和析构函数实现 ====================
CommandExecutor::CommandExecutor(CommandRegistry &registry) : m_Registry(registry)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Executor");
  m_Logger->debug("CommandExecutor created");

  // 创建默认上下文用于编辑模式
  CreateDefaultExecutionContext(CommandContextFlags::CONTEXT_EDITOR,
                                "Mite Default Editor Command Execution Context");
}
CommandExecutor::~CommandExecutor()
{
  // 等待所有任务完成
  Stop(true);
  m_Logger->debug("CommandExecutor destroyed");
}
// ==================== 执行器状态管理接口实现 ====================
void CommandExecutor::Start()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (!m_IsRunning) {
    m_IsRunning = true;
    m_Logger->info("CommandExecutor started");
  }
}
void CommandExecutor::Stop(bool waitForCompletion)
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_IsRunning)
      return;
    m_IsRunning = false;
  }
  if (waitForCompletion) {
    // 等待所有任务完成
    for (auto &future : m_TaskFutures) {
      if (future.valid()) {
        try {
          future.wait();
        }
        catch (const std::exception &e) {
          m_Logger->error("Error waiting for command completion: {}", e.what());
        }
      }
    }
    m_TaskFutures.clear();
  }
  // 即便waitForCompletion=false，BSThread也会逐个执行完毕各个子线程，这里只是提前退出而已
  m_Logger->info("CommandExecutor stopped");
}
bool CommandExecutor::IsRunning() const
{
  return m_IsRunning;
}
// ==================== 命令提交接口实现 ====================
bool CommandExecutor::SubmitCommandAsync(CommandHandle handle,
                                         CommandExecutionContext *context,
                                         CommandPriority priority)
{
  // 检查命令本身
  if (!handle.IsValid()) {
    m_Logger->warn("Attempted to submit invalid command handle");
    return false;
  }
  std::lock_guard<std::mutex> lock(m_Mutex);
  // 若并非在System的Start()和Stop()窗口提交，则提交失败（此时并非运行状态）
  if (!m_IsRunning) {
    m_Logger->warn("Cannot submit command - executor is not running");
    return false;
  }
  // 若在System的Pause()和Resume()窗口提交，则提交失败（此时为暂停状态）
  if (m_IsPaused) {
    m_Logger->warn("Cannot submit command - executor is paused");
    return false;
  }
  auto *execContext = context ? context : GetDefaultExecutionContext();
  // 检查执行上下文的可用性
  if (!execContext) {
    m_Logger->warn("No execution context available for command handle: {}", handle.ToString());
    return false;
  }
  // 检查命令可用性（基于句柄）
  if (!execContext->IsCommandAvailable(m_Registry, handle)) {
    m_Logger->warn("Command handle {} is not available in context '{}'",
                   handle.ToString(),
                   execContext->GetName());
    return false;
  }

  // 获取命令类型信息用于类型安全检查
  const Command *command = m_Registry.PeekCommand(handle);
  if (!command) {
    m_Logger->warn("Command handle not found: {}", handle.ToString());
    return false;
  }
  std::type_index expectedType = typeid(*command);

  // 使用线程池提交任务
  auto &threadPool = ThreadPoolManager::GetDefaultPool();

  // 提交给线程池执行命令
  auto taskFuture = threadPool.submit_task(
      [this, task = CommandTask(handle, execContext, expectedType)]() mutable {
        ExecuteSingleCommand(std::move(task));
      },
      static_cast<BS::priority_t>(priority));

  // 管理taskFuture
  m_TaskFutures.push_back(std::move(taskFuture));
  m_TotalCommands++;
  m_ExecutingCommands++;
  m_Logger->debug("Command handle {} submitted for execution in context '{}' with priority {}",
                  handle.ToString(),
                  execContext->GetName(),
                  static_cast<int>(priority));
  return true;
}
CommandResult CommandExecutor::ExecuteCommand(CommandHandle handle,
                                              CommandExecutionContext *context)
{
  // 检查命令本身
  if (!handle.IsValid()) {
    return CommandResult::Failure("Invalid command handle");
  }
  // 检查执行上下文的可用性
  auto *execContext = context ? context : GetDefaultExecutionContext();
  if (!execContext) {
    return CommandResult::Failure("No execution context available");
  }
  // 检查命令可用性
  if (!execContext->IsCommandAvailable(m_Registry, handle)) {
    m_Logger->warn("Command handle {} is not available in context '{}'",
                   handle.ToString(),
                   execContext->GetName());
    return CommandResult::Failure("Command not available in current context");
  }

  // 获取命令类型信息用于类型安全检查
  const Command *command = m_Registry.PeekCommand(handle);
  if (!command) {
    return CommandResult::Failure("Command handle not found: " + handle.ToString());
  }
  std::type_index expectedType = typeid(*command);

  // 使用命令和上下文创建Task
  CommandTask task(handle, execContext, expectedType);
  m_Logger->debug("Executing command handle {} synchronously in context '{}'",
                  handle.ToString(),
                  execContext->GetName());

  // 主线程内执行单次命令
  return ExecuteSingleCommand(std::move(task));
}
size_t CommandExecutor::SubmitCommands(const std::vector<CommandHandle> &handles,
                                       CommandExecutionContext *context,
                                       CommandPriority priority)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  // 若并非在System的Start()和Stop()窗口提交，则提交失败（此时并非运行状态）
  if (!m_IsRunning) {
    m_Logger->warn("Cannot submit commands - executor is not running");
    return 0;
  }
  // 若在System的Pause()和Resume()窗口提交，则提交失败（此时为暂停状态）
  if (m_IsPaused) {
    m_Logger->warn("Cannot submit commands - executor is paused");
    return 0;
  }
  // 检查命令可用性
  auto *execContext = context ? context : GetDefaultExecutionContext();
  if (!execContext) {
    m_Logger->warn("No execution context available for batch submission");
    return 0;
  }

  auto &threadPool = ThreadPoolManager::GetDefaultPool();
  size_t submittedCount = 0;

  for (const auto &handle : handles) {
    if (!handle.IsValid())
      continue;
    if (!execContext->IsCommandAvailable(m_Registry, handle)) {
      m_Logger->warn("Command handle {} not available in context", handle.ToString());
      continue;
    }
    // 获取命令类型信息用于类型安全检查
    const Command *command = m_Registry.PeekCommand(handle);
    if (!command) {
      m_Logger->warn("Command handle not found: {}", handle.ToString());
      continue;
    }
    std::type_index expectedType = typeid(*command);

    // 使用命令和上下文创建Task,子线程提交任务
    auto future = threadPool.submit_task(
        [this, task = CommandTask(handle, execContext, expectedType)]() mutable {
          ExecuteSingleCommand(std::move(task));
        },
        static_cast<BS::priority_t>(priority));
    m_TaskFutures.push_back(std::move(future));
    submittedCount++;
    m_TotalCommands++;
    m_ExecutingCommands++;
  }

  if (submittedCount > 0) {
    m_Logger->debug("Submitted {} command handles for execution in context '{}' with priority {}",
                    submittedCount,
                    execContext->GetName(),
                    static_cast<int>(priority));
  }
  return submittedCount;
}
// ==================== 执行上下文管理接口实现 ====================
void CommandExecutor::SetDefaultExecutionContext(ExecutionContextPtr context)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_DefaultContext = std::move(context);
}
CommandExecutionContext *CommandExecutor::GetDefaultExecutionContext() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_DefaultContext.get();
}
CommandExecutionContext *CommandExecutor::CreateDefaultExecutionContext(
    CommandContextFlags contextFlags, const std::string &name)
{
  auto context = std::make_unique<CommandExecutionContext>(contextFlags, name);
  auto *contextPtr = context.get();
  SetDefaultExecutionContext(std::move(context));
  return contextPtr;
}
// ==================== 执行统计接口实现 ====================
// size_t CommandExecutor::GetPendingCommandCount() const
//{
//  // BS线程池内部管理队列，无法直接获取
//  return 0;
//}
size_t CommandExecutor::GetExecutingCommandCount() const
{
  return m_ExecutingCommands.load();
}
size_t CommandExecutor::GetCompletedCommandCount() const
{
  return m_CompletedCommands.load();
}
size_t CommandExecutor::GetTotalCommandCount() const
{
  return m_TotalCommands.load();
}
// ==================== 执行控制接口实现 ====================
void CommandExecutor::Pause()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_IsPaused = true;
  m_Logger->info("CommandExecutor paused");
}
void CommandExecutor::Resume()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_IsPaused = false;
  m_Logger->info("CommandExecutor resumed");
}
bool CommandExecutor::IsPaused() const
{
  return m_IsPaused.load();
}
size_t CommandExecutor::ClearPendingCommands()
{
  // BS线程池内部管理队列，无法直接清空
  // 只能停止新任务的提交，等待现有任务完成
  //
  // 原因：
  // BS::thread_pool 的限制。BS线程池遵循"任务完成"模型，
  // 一旦任务提交，就保证会执行完成。而安全地中断线程，
  // 需要复杂的协调机制，容易引入竞态条件。
  //
  // 解决方案：
  // 子线程尽可能执行简单命令，复杂命令拆分后分发。
  // 确保命令短时间内可以执行完成，用户不感知到Pending即可
  m_Logger->warn("ClearPendingCommands not supported with BS thread pool");
  return 0;
}
// ==================== 单个命令执行实现 ====================
CommandResult CommandExecutor::ExecuteSingleCommand(CommandTask task)
{
  // 1. 执行前阶段
  if (!task.handle.IsValid() || !task.context) {
    m_ExecutingCommands--;
    return CommandResult::Failure("Invalid command handle or context");
  }

  // 1.1. 从注册表获取命令对象（转移所有权）
  CommandPtr command = m_Registry.AcquireCommand(task.handle, task.expectedType);
  if (!command) {
    m_ExecutingCommands--;
    return CommandResult::Failure("Failed to acquire command from registry: " +
                                  task.handle.ToString());
  }
  std::string commandName = command->GetName();

  // 1.2. 变更CommandExecutionState状态
  CommandExecutionState preExecutingState;
  switch (m_Registry.GetCommandState(task.handle)) {
    case CommandExecutionState::PENDING:
      preExecutingState = CommandExecutionState::EXECUTING;  // 命令等待执行（可执行）
      break;
    case CommandExecutionState::SUCCEEDED:
      preExecutingState = CommandExecutionState::UNDOING;  // 命令执行成功（可撤销）
      break;
    case CommandExecutionState::UNDONE:
      preExecutingState = CommandExecutionState::REDOING;  // 命令已被撤销（可重做）
      break;
    case CommandExecutionState::REDONE:
      preExecutingState = CommandExecutionState::UNDOING;  // 命令已被重做（可再次撤销）
      break;
    default:
      return CommandResult::Failure("Invalid Command Execution State");  // 与情况不符
  }
  m_Registry.SetCommandState(task.handle, preExecutingState);

  // 1.3. 发布执行开始事件
  EventBus::Publish<CommandExecuteEvent>(CommandExecuteEvent(command.get(), task.context));

  // 1.4. 记录开始执行
  task.context->RecordCommandExecutionStart(task.handle);
  m_Logger->info("Executing command '{}' (handle: {}) in context '{}'",
                 commandName,
                 task.handle.ToString(),
                 task.context->GetName());

  // 2. 执行阶段
  CommandResult result;
  try {
    switch (m_Registry.GetCommandState(task.handle)) {
      case CommandExecutionState::EXECUTING:
        result = command->Execute();  // 执行命令
        break;
      case CommandExecutionState::UNDOING:
        result = command->Undo();  // 执行撤销
        break;
      case CommandExecutionState::REDOING:
        result = command->Execute();  // 执行重做
        break;
      default:
        result = CommandResult::Failure("Invalid Command Execution State");  // 与情况不符
    }
  }
  catch (const std::exception &e) {
    m_Logger->error("Command '{}' execution failed with exception: {}", commandName, e.what());
    result = CommandResult::Failure(std::string("Exception: ") + e.what());
  }
  catch (...) {
    m_Logger->error("Command '{}' execution failed with unknown exception", commandName);
    result = CommandResult::Failure("Unknown exception");
  }

  // 3. 执行完成阶段

  // 3.1. 记录执行完成
  task.context->RecordCommandExecutionComplete(task.handle, result);

  // 3.2. 变更CommandExecutionState状态
  CommandExecutionState postExecutingState;
  if (result.success) {
    switch (m_Registry.GetCommandState(task.handle)) {
      case CommandExecutionState::EXECUTING:
        postExecutingState = CommandExecutionState::SUCCEEDED;  // 命令执行成功
        break;
      case CommandExecutionState::UNDOING:
        postExecutingState = CommandExecutionState::UNDONE;  // 命令撤销成功
        break;
      case CommandExecutionState::REDOING:
        postExecutingState = CommandExecutionState::REDONE;  // 命令重做撤销
        break;
      default:
        return CommandResult::Failure("Invalid Command Execution State");  // 与情况不符
    }
  }
  else {
    postExecutingState = CommandExecutionState::FAILED;
  }
  m_Registry.SetCommandState(task.handle, postExecutingState);

  // 3.3. 如果执行成功，将命令重新存储到注册表
  if (result.success && command->CanUndo()) {
    // 重新存储到原句柄（不改变CommandExecutionState状态。状态与Store解耦）
    if (m_Registry.ReStoreCommand(task.handle, std::move(command))) {
      // 发布事件，使用原句柄
      result.commandHandle = task.handle;
      CommandCompletedEvent event(result);
      EventBus::Publish(event);

      m_Logger->debug("Command re-stored to original handle: {}", task.handle.ToString());
    }
    else {
      m_Logger->error("Command re-stored failed: {}", result.commandHandle.ToString());
    }
  }

  // 3.4. 更新统计
  m_ExecutingCommands--;
  m_CompletedCommands++;
  m_Logger->info("Command '{}' completed with result: {}",
                 commandName,
                 result.success ? "success" : "failure");
  return result;
}
}  // namespace mite