#include "command_system.h"

namespace mite {
// ==================== 单例实现 ====================
CommandSystem &CommandSystem::Get() {
  static CommandSystem instance;
  return instance;
}

CommandSystem::CommandSystem()
    : m_Registry(std::make_unique<CommandRegistry>()),
      m_Executor(std::make_unique<CommandExecutor>(*m_Registry)),
      m_UndoStack(std::make_unique<CommandUndoStack>()),
      m_RedoStack(std::make_unique<CommandRedoStack>()) {
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command System");
  m_Logger->debug("Command System created");
}
CommandSystem::~CommandSystem() { Shutdown(true); }

// ==================== 系统生命周期管理接口实现 ====================
void CommandSystem::Initialize(const CommandSystemInitParams &initParams) {
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
  m_UndoStack->SetMaxSize(initParams.maxUndoStackSize);
  m_RedoStack->SetMaxSize(initParams.maxRedoStackSize);

  // 启动执行器
  m_Executor->Start();

  m_initialized = true;
  m_Logger->info("CommandSystem initialized");
}

void CommandSystem::Shutdown(bool waitForCompletion) {
  if (!m_initialized) return;

  // 停止执行器
  m_Executor->Stop(waitForCompletion);

  // 清空栈和上下文
  ClearStacks();
  m_initialized = false;
  m_Logger->info("CommandSystem shutdown");
}

bool CommandSystem::IsInitialized() const { return m_initialized; }

bool CommandSystem::HasCommand(const CommandHandle &handle) const {
  return m_Registry->HasCommand(handle);
}

bool CommandSystem::ReleaseCommand(const CommandHandle &handle) {
  return m_Registry->ReleaseCommand(handle);
}

// ==================== 命令执行接口实现 ====================
CommandResult CommandSystem::Execute(CommandHandle handle) {
  if (!handle.IsValid()) {
    return CommandResult::Failure("Invalid command handle");
  }
  // 执行命令
  CommandResult result = m_Executor->ExecuteCommand(handle);

  // 注意：撤销栈的维护现在通过事件处理，这里不再直接处理
  return result;
}

CommandResult CommandSystem::Submit(CommandHandle handle,
                                    CommandPriority priority) {
  if (!handle.IsValid()) {
    return CommandResult::Failure("Invalid command handle");
  }
  // 异步提交
  bool submitted = m_Executor->SubmitCommandAsync(handle, nullptr, priority);
  if (submitted) {
    return CommandResult::Pending("Command submitted for execution");
  } else {
    return CommandResult::Failure("Command submission failed");
  }
}
// ==================== Undo/Redo 接口实现 ====================
CommandResult CommandSystem::Undo() {
  if (!CanUndo()) {
    return CommandResult::Failure("Nothing to undo");
  }
  // 从撤销栈获取句柄
  CommandHandle handle = m_UndoStack->Pop();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to pop from undo stack");
  }
  // 使用default上下文执行撤销操作
  auto result = m_Executor->ExecuteCommand(handle);

  // 注意：重做栈的维护通过事件处理，这里不再直接处理
  return result;
}

bool CommandSystem::UndoSubmit() {
  if (!CanUndo()) {
    return false;
  }
  // 从撤销栈获取句柄
  CommandHandle handle = m_UndoStack->Pop();
  if (!handle.IsValid()) {
    return false;
  }
  // 使用default上下文执行异步撤销操作
  return m_Executor->SubmitCommandAsync(handle);
}

CommandResult CommandSystem::Redo() {
  if (!CanRedo()) {
    return CommandResult::Failure("Nothing to redo");
  }
  // 从重做栈获取句柄
  CommandHandle handle = m_RedoStack->Pop();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to pop from redo stack");
  }
  // 使用default上下文执行重做操作
  auto result = m_Executor->ExecuteCommand(handle);

  // 注意：撤销栈的维护通过事件处理，这里不再直接处理
  return result;
}

bool CommandSystem::RedoSubmit() {
  if (!CanRedo()) {
    return false;
  }
  // 从重做栈获取句柄
  CommandHandle handle = m_RedoStack->Pop();
  if (!handle.IsValid()) {
    return false;
  }
  // 使用default上下文执行异步重做操作
  return m_Executor->SubmitCommandAsync(handle);
}

bool CommandSystem::CanUndo() const {
  // 栈不为空，即可撤销
  return !m_UndoStack->IsEmpty();
}
bool CommandSystem::CanRedo() const {
  // 栈不为空，即可重做
  return !m_RedoStack->IsEmpty();
}
size_t CommandSystem::GetUndoStackSize() const {
  return m_UndoStack->GetSize();
}
size_t CommandSystem::GetRedoStackSize() const {
  return m_RedoStack->GetSize();
}
void CommandSystem::ClearStacks() {
  m_UndoStack->Clear();
  m_RedoStack->Clear();
}
void CommandSystem::SetMaxStackSize(size_t maxSize) {
  m_UndoStack->SetMaxSize(maxSize);
  m_RedoStack->SetMaxSize(maxSize);
}

// ==================== 其他接口实现 ====================
CommandExecutor &CommandSystem::GetExecutor() { return *m_Executor; }
CommandRegistry &CommandSystem::GetRegistry() { return *m_Registry; }
CommandUndoStack &CommandSystem::GetUndoStack() { return *m_UndoStack; }
CommandRedoStack &CommandSystem::GetRedoStack() { return *m_RedoStack; }

void CommandSystem::OnCommandCompleted(CommandCompletedEvent &event) {
  if (event.GetResult().success && event.HasCommandHandle()) {
    CommandHandle handle = event.GetCommandHandle();

    // 根据事件类型决定如何维护栈
    if (event.GetResult().state == CommandExecutionState::SUCCEEDED) {
      // 正常命令执行完成：添加到撤销栈，清空重做栈
      m_UndoStack->Push(*m_Registry, handle);
      m_RedoStack->Clear();
      m_Logger->debug("Command handle {} added to undo stack",
                      handle.ToString());
    } else if (event.GetResult().state == CommandExecutionState::UNDONE) {
      // 撤销操作完成：添加到重做栈
      m_RedoStack->Push(handle);
      m_Logger->debug("Command handle {} added to redo stack (undo completed)",
                      handle.ToString());
    } else if (event.GetResult().state == CommandExecutionState::REDONE) {
      // 重做操作完成：添加到撤销栈
      m_UndoStack->Push(*m_Registry, handle);
      m_Logger->debug("Command handle {} added to undo stack (redo completed)",
                      handle.ToString());
    }
  }
  // Failed则无需处理
}
}  // namespace mite