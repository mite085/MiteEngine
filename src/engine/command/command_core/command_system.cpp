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

  // 订阅命令结束事件，作为维护撤销栈的唯一依据
  // Immediate立即模式
  // 命令完成可能是在子线程内
  m_EventSubscriptions.SubscribeImmediate<CommandCompletedEvent>(
      BIND_DISPATCH_FN(OnCommandCompleted), EventPriority::Normal);

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

   // 清空栈和上下文
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
CommandResult CommandSystem::Execute(CommandHandle handle, const std::string &contextName)
{
  if (!handle.IsValid()) {
    return CommandResult::Failure("Invalid command handle");
  }
  auto *context = GetContext(contextName);
  if (!context) {
    return CommandResult::Failure("Context not found: " + contextName);
  }
  // 检查命令可用性
  if (!context->IsCommandAvailable(handle)) {
    return CommandResult::Failure("Command not available in context: " + contextName);
  }
  // 执行命令
  CommandResult result = m_executor->ExecuteCommand(handle, context);

  // 注意：撤销栈的维护现在通过事件处理，这里不再直接处理
  return result;
}

CommandResult CommandSystem::Submit(CommandHandle handle,
                                    const std::string &contextName,
                                    CommandPriority priority)
{
  if (!handle.IsValid()) {
    return CommandResult::Failure("Invalid command handle");
  }
  auto *context = GetContext(contextName);
  if (!context) {
    m_Logger->warn("Context not found: {}", contextName);
    return CommandResult::Failure("Context not found");
  }
  if (!context->IsCommandAvailable(handle)) {
    m_Logger->warn(
        "Command handle {} not available in context: {}", handle.ToString(), contextName);
    return CommandResult::Failure("Command not available in context");
  }
  // 异步提交
  bool submitted = m_executor->SubmitCommandAsync(handle, context, priority);
  if (submitted) {
    return CommandResult::Pending("Command submitted for execution");
  }
  else {
    return CommandResult::Failure("Command submission failed");
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
  // 从撤销栈获取句柄
  CommandHandle handle = m_undoStack->Pop();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to pop from undo stack");
  }
  // 使用default上下文执行撤销操作
  auto result = m_executor->ExecuteCommand(handle, GetContext("Default"));

  // 注意：重做栈的维护通过事件处理，这里不再直接处理
  return result;
}

bool CommandSystem::UndoSubmit()
{
  if (!CanUndo()) {
    return false;
  }
  // 从撤销栈获取句柄
  CommandHandle handle = m_undoStack->Pop();
  if (!handle.IsValid()) {
    return false;
  }
  // 使用default上下文执行异步撤销操作
  return m_executor->SubmitCommandAsync(handle, GetContext("Default"));
}

CommandResult CommandSystem::Redo()
{
  if (!CanRedo()) {
    return CommandResult::Failure("Nothing to redo");
  }
  // 从重做栈获取句柄
  CommandHandle handle = m_redoStack->Pop();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to pop from redo stack");
  }
  // 使用default上下文执行重做操作
  auto result = m_executor->ExecuteCommand(handle, GetContext("Default"));

  // 注意：撤销栈的维护通过事件处理，这里不再直接处理
  return result;
}

bool CommandSystem::RedoSubmit()
{
  if (!CanRedo()) {
    return false;
  }
  // 从重做栈获取句柄
  CommandHandle handle = m_redoStack->Pop();
  if (!handle.IsValid()) {
    return false;
  }  
  // 使用default上下文执行异步重做操作
  return m_executor->SubmitCommandAsync(handle, GetContext("Default"));
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
  if (event.GetResult().success && event.HasCommandHandle()) {
    CommandHandle handle = event.GetCommandHandle();

    // 根据事件类型决定如何维护栈
    if (event.GetResult().state == CommandExecutionState::SUCCEEDED) {
      // 正常命令执行完成：添加到撤销栈，清空重做栈
      m_undoStack->Push(handle);
      m_redoStack->Clear();
      m_Logger->debug("Command handle {} added to undo stack", handle.ToString());
    }
    else if (event.GetResult().state == CommandExecutionState::UNDONE) {
      // 撤销操作完成：添加到重做栈
      m_redoStack->Push(handle);
      m_Logger->debug("Command handle {} added to redo stack (undo completed)", handle.ToString());
    }
    else if (event.GetResult().state == CommandExecutionState::REDONE) {
      // 重做操作完成：添加到撤销栈
      m_undoStack->Push(handle);
      m_Logger->debug("Command handle {} added to undo stack (redo completed)", handle.ToString());
    }
  }
  // Failed则无需处理
}

}  // namespace mite