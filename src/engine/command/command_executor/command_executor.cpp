#include "command_executor.h"
#include "command_core/command_event.h"

namespace mite {
// ==================== 构造函数和析构函数实现 ====================
CommandExecutor::CommandExecutor()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Executor");
  m_Logger->debug("CommandExecutor created");
}
CommandExecutor::~CommandExecutor()
{
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

  m_Logger->info("CommandExecutor stopped");
}
bool CommandExecutor::IsRunning() const
{
  return m_IsRunning;
}
// ==================== 命令提交接口实现 ====================
bool CommandExecutor::SubmitCommandAsync(CommandPtr command,
                                         CommandExecutionContext *context,
                                         BS::priority_t priority)
{
  // 检查命令本身
  if (!command) {
    m_Logger->warn("Attempted to submit null command");
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
    m_Logger->warn("No execution context available for command '{}'", command->GetName());
    return false;
  }
  // 检查命令可用性
  if (!execContext->IsCommandAvailable(command.get())) {
    m_Logger->warn("Command '{}' is not available in context '{}'",
                 command->GetName(),
                 execContext->GetName());
    return false;
  }

  // 使用线程池提交任务
  auto &threadPool = ThreadPoolManager::GetDefaultPool();

  // 用共享指针保持命令对象存活直到执行完成
  auto commandShared = std::make_shared<CommandPtr>(std::move(command));

  auto taskFuture = threadPool.submit_task(
      [this, cmdPtr = commandShared, execContext]() mutable {
        if (!*cmdPtr)
          return;

        // 使用命令和上下文创建Task，执行单次命令
        CommandTask task(std::move(*cmdPtr), execContext);
        auto result = ExecuteSingleCommand(std::move(task));
      },
      priority);

  m_TaskFutures.push_back(std::move(taskFuture));
  m_TotalCommands++;
  m_ExecutingCommands++;

  m_Logger->debug("Command '{}' submitted for execution in context '{}' with priority {}",
                  (*commandShared)->GetName(),
                  execContext->GetName(),
                  static_cast<int>(priority));
  return true;
}
CommandResult CommandExecutor::ExecuteCommand(CommandPtr command, CommandExecutionContext *context)
{
  // 检查命令本身
  if (!command)
    return CommandResult::Failure("Invalid command");
  // 检查执行上下文的可用性
  auto *execContext = context ? context : GetDefaultExecutionContext();
  if (!execContext) {
    return CommandResult::Failure("No execution context available");
  }
  // 检查命令可用性
  if (!execContext->IsCommandAvailable(command.get())) {
    m_Logger->warn("Command '{}' is not available in context '{}'",
                   command->GetName(),
                   execContext->GetName());
    return CommandResult::Failure("Command not available in current context");
  }

  // 使用命令和上下文创建Task
  CommandTask task(std::move(command), execContext);

  m_Logger->debug("Executing command '{}' synchronously in context '{}'",
                  task.command->GetName(),
                  task.context->GetName());

  // 主线程内执行单次命令
  return ExecuteSingleCommand(std::move(task));
}
size_t CommandExecutor::SubmitCommands(std::vector<CommandPtr> commands,
                                       CommandExecutionContext *context,
                                       BS::priority_t priority)
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

  auto *execContext = context ? context : GetDefaultExecutionContext();
  if (!execContext) {
    m_Logger->warn("No execution context available for batch submission");
    return 0;
  }

  auto &threadPool = ThreadPoolManager::GetDefaultPool();
  size_t submittedCount = 0;

  for (auto &command : commands) {
    if (!command)
      continue;

    // 使用命令和上下文创建Task
    CommandTask task(std::move(command), execContext);

    // 子线程提交任务
    auto future = threadPool.submit_task(
        [this, task = std::move(task)]() mutable {
          // 执行单次命令
          auto result = ExecuteSingleCommand(std::move(task));  
        },
        priority);

    m_TaskFutures.push_back(std::move(future));
    submittedCount++;
    m_TotalCommands++;
    m_ExecutingCommands++;
  }

  if (submittedCount > 0) {
    m_Logger->debug("Submitted {} commands for execution in context '{}' with priority {}",
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
CommandExecutionContext *CommandExecutor::CreateDefaultExecutionContext(uint32_t contextFlags,
                                                                        const std::string &name)
{
  auto context = std::make_unique<CommandExecutionContext>(contextFlags, name);
  auto *contextPtr = context.get();
  SetDefaultExecutionContext(std::move(context));
  return contextPtr;
}
// ==================== 执行统计接口实现 ====================
//size_t CommandExecutor::GetPendingCommandCount() const
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
  m_Logger->warn("ClearPendingCommands not supported with BS thread pool");
  return 0;
}
// ==================== 单个命令执行实现 ====================
CommandResult CommandExecutor::ExecuteSingleCommand(CommandTask task)
{
  if (!task.command || !task.context) {
    m_ExecutingCommands--;
    return CommandResult::Failure("Invalid command or context");
  }

  // 检查命令是否在上下文中可用
  if (!task.context->IsCommandAvailable(task.command.get())) {
    m_Logger->warn("Command '{}' is not available in context '{}'",
                   task.command->GetName(),
                   task.context->GetName());
    m_ExecutingCommands--;
    return CommandResult::Failure("Command not available in current context");
  }

  // 记录执行开始
  task.context->RecordCommandExecutionStart(task.command.get());

  // 发布执行开始事件
  EventBus::Publish<CommandExecuteEvent>(
      CommandExecuteEvent(task.command.get(), task.context, task.command->GetPriority()));

  m_Logger->info(
      "Executing command '{}' in context '{}'", task.command->GetName(), task.context->GetName());

  // 执行命令
  CommandResult result;
  try {
    result = task.command->Execute();
  }
  catch (const std::exception &e) {
    m_Logger->error(
        "Command '{}' execution failed with exception: {}", task.command->GetName(), e.what());
    result = CommandResult::Failure(std::string("Exception: ") + e.what());
  }
  catch (...) {
    m_Logger->error("Command '{}' execution failed with unknown exception",
                    task.command->GetName());
    result = CommandResult::Failure("Unknown exception");
  }

  // 记录执行完成
  task.context->RecordCommandExecutionComplete(task.command.get(), result);

  // 发布执行完成事件
  EventBus::Publish<CommandCompletedEvent>(CommandCompletedEvent(std::move(result)));

  // 更新统计
  m_ExecutingCommands--;
  m_CompletedCommands++;

  m_Logger->info("Command '{}' completed with result: {}",
                 task.command->GetName(),
                 result.success ? "success" : "failure");

  // 返回包含命令的Result 
  return CommandResult(result.success, result.state, result.message, std::move(task.command));
}
}  // namespace mite