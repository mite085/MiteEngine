#include "command_system.h"

namespace mite {

// ==================== 单例实现 ====================
CommandSystem &CommandSystem::Get()
{
  static CommandSystem instance;
  return instance;
}

CommandSystem::CommandSystem()
    : m_executor(std::make_unique<CommandExecutor>()),
      m_undoStack(std::make_unique<CommandUndoStack>()),
      m_redoStack(std::make_unique<CommandRedoStack>())
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command System");
  m_Logger->debug("Command System created");

  // 订阅命令结束事件，作为维护撤销栈的唯一依据
  // Immediate立即模式
  // 命令完成可能是在子线程内
  m_EventSubscriptions.SubscribeImmediate<CommandCompletedEvent>(
      BIND_DISPATCH_FN(OnCommandCompleted),
      EventPriority::Normal
  );
}
CommandSystem::~CommandSystem()
{
  Shutdown(true);
}

// ==================== 系统生命周期管理接口实现 ====================
void CommandSystem::Initialize(const CommandSystemInitParams &initParams)
{
  if (m_initialized) {
    m_Logger->warn("CommandSystem already initialized");
    return;
  }

  // 配置栈大小
  m_undoStack->SetMaxSize(initParams.maxUndoStackSize);
  m_redoStack->SetMaxSize(initParams.maxRedoStackSize);

  // 启动执行器
  m_executor->Start();

  // 创建默认上下文
  GetOrCreateContext("Default", CONTEXT_NONE);

  m_initialized = true;
  m_Logger->info("CommandSystem initialized");
}

void CommandSystem::Shutdown(bool waitForCompletion)
{
  if (!m_initialized)
    return;

  // 停止执行器
  m_executor->Stop(waitForCompletion);

  // 清空栈
  ClearStacks();
  m_contexts.clear();
  m_initialized = false;
  m_Logger->info("CommandSystem shutdown");
}

bool CommandSystem::IsInitialized() const
{
  return m_initialized;
}

// ==================== 命令执行接口实现 ====================
CommandResult CommandSystem::Execute(CommandPtr command, const std::string &contextName)
{
  if (!command) {
    return CommandResult::Failure("Invalid command");
  }

  auto *context = GetContext(contextName);
  if (!context) {
    return CommandResult::Failure("Context not found: " + contextName);
  }

  // 检查命令可用性
  if (!context->IsCommandAvailable(command.get())) {
    return CommandResult::Failure("Command not available in context: " + contextName);
  }

  // 执行命令
  CommandResult result = m_executor->ExecuteCommand(std::move(command), context);

  // 如果执行成功且命令可撤销，添加到撤销栈
  if (result.success && result.command->CanUndo()) {
    m_undoStack->Push(std::move(result.command));
    m_redoStack->Clear();  // 执行新命令时清空重做栈
  }

  return result;
}

CommandResult CommandSystem::Submit(CommandPtr command,
                                    const std::string &contextName,
                                    BS::priority_t priority)
{
  if (!command)
    return CommandResult::Failure("Invalid command");

  auto *context = GetContext(contextName);
  if (!context) {
    m_Logger->warn("Context not found: {}", contextName);
    return CommandResult::Failure("Context not found");
  }

  if (!context->IsCommandAvailable(command.get())) {
    m_Logger->warn("Command not available in context: {}", contextName);
    return CommandResult::Failure("Command not available in context");
  }

  // 异步提交
  bool submited = m_executor->SubmitCommandAsync(std::move(command), context, priority);
  if (submited) {
    // 成功提交，返回Pending状态
    return CommandResult::Pending(std::string("Command Submit pending"));
  }
  else {
    // 提交失败
    return CommandResult::Failure(std::string("Command Submit failed"));
  }
}

// ==================== 上下文管理接口实现 ====================
CommandExecutionContext *CommandSystem::GetOrCreateContext(const std::string &name,
                                                           CommandContextFlags contextFlags)
{
  auto it = m_contexts.find(name);
  if (it != m_contexts.end()) {
    return it->second.get();
  }

  // 创建上下文
  auto context = std::make_unique<CommandExecutionContext>(contextFlags, name);
  auto *contextPtr = context.get();
  m_contexts[name] = std::move(context);

  return contextPtr;
}

CommandExecutionContext *CommandSystem::GetContext(const std::string &name) const
{
  auto it = m_contexts.find(name);
  return it != m_contexts.end() ? it->second.get() : nullptr;
}

void CommandSystem::RemoveContext(const std::string &name)
{
  m_contexts.erase(name);
}

// ==================== Undo/Redo 接口实现 ====================
CommandResult CommandSystem::Undo()
{
  if (!CanUndo()) {
    return CommandResult::Failure("Nothing to undo");
  }

  // 获取“撤销”的命令
  auto command = m_undoStack->Pop();
  if (!command) {
    return CommandResult::Failure("Failed to pop from undo stack");
  }

  // 执行撤销
  auto result = m_executor->ExecuteCommand(std::move(command), GetContext("Default"));
  if (result.success) {
    // 将撤销后的命令移至“重做”中
    m_redoStack->Push(std::move(result.command));
  }

  return result;
}

CommandResult CommandSystem::Redo()
{
  if (!CanRedo()) {
    return CommandResult::Failure("Nothing to redo");
  }

  // 获取“重做”的命令
  auto command = m_redoStack->Pop();
  if (!command) {
    return CommandResult::Failure("Failed to pop from redo stack");
  }

  // 执行重做
  auto result = m_executor->ExecuteCommand(std::move(command), GetContext("Default"));
  if (result.success) {
    // 将撤销后的命令移至“撤销”中
    m_undoStack->Push(std::move(result.command));
  }

  return result;
}

bool CommandSystem::CanUndo() const
{
  // 栈不为空，即可撤销
  return !m_undoStack->IsEmpty();
}
bool CommandSystem::CanRedo() const
{
  // 栈不为空，即可重做
  return !m_redoStack->IsEmpty();
}
size_t CommandSystem::GetUndoStackSize() const
{
  return m_undoStack->GetSize();
}
size_t CommandSystem::GetRedoStackSize() const
{
  return m_redoStack->GetSize();
}
void CommandSystem::ClearStacks()
{
  m_undoStack->Clear();
  m_redoStack->Clear();
}
void CommandSystem::SetMaxStackSize(size_t maxSize)
{
  m_undoStack->SetMaxSize(maxSize);
  m_redoStack->SetMaxSize(maxSize);
}

// ==================== 其他接口实现 ====================
CommandExecutor &CommandSystem::GetExecutor()
{
  return *m_executor;
}
CommandFactory &CommandSystem::GetFactory()
{
  return CommandFactory::Get();
}
CommandRegistry &CommandSystem::GetRegistry()
{
  return CommandRegistry::Get();
}
CommandUndoStack &CommandSystem::GetUndoStack()
{
  return *m_undoStack;
}
CommandRedoStack &CommandSystem::GetRedoStack()
{
  return *m_redoStack;
}

void CommandSystem::OnCommandCompleted(CommandCompletedEvent &event)
{
  if (event.IsSuccess() && event.HasCommand()) {
    // 从事件中获取命令对象所有权
    auto command = event.ReleaseCommand();

    // 判断是否可以撤销
    if (command && command->CanUndo()) {
      // 添加到撤销栈
      GetUndoStack().Push(std::move(command));
      // 清空重做栈
      GetRedoStack().Clear();

      m_Logger->debug("Command '{}' added to undo stack", command->GetName());
    }
  }
}

}  // namespace mite