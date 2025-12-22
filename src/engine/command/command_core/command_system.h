#ifndef MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
#define MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM

#include "command_core/command.h"
#include "command_core/command_event.h"
#include "command_core/command_registry.h"
#include "command_executor/command_execution_context.h"
#include "command_executor/command_executor.h"
#include "command_executor/command_redo_stack.h"
#include "command_executor/command_undo_stack.h"

namespace mite {
/**
 * @brief 命令系统 - 基于CommandHandle的统一管理
 *
 * 职责：
 * 1. 作为统一接口，封装命令相关操作
 * 2. 订阅命令完成事件，维护撤销/重做栈
 * 3. 管理执行上下文
 */
class CommandSystem {
 public:
  // ==================== 单例访问接口 ====================
  static CommandSystem &Get();

  // ==================== 系统生命周期管理接口 ====================
  // 初始化参数结构体
  struct CommandSystemInitParams {
    size_t maxUndoStackSize = 100;
    size_t maxRedoStackSize = 100;

    // 添加默认构造函数 + 带参数的构造函数
    CommandSystemInitParams() = default;
    CommandSystemInitParams(size_t undoSize, size_t redoSize)
        : maxUndoStackSize(undoSize), maxRedoStackSize(redoSize) {}
  };
  /**
   * @brief 初始化命令系统
   * @param initParams 初始化参数
   */
  void Initialize(const CommandSystemInitParams &initParams);
  /**
   * @brief 关闭命令系统
   * @param waitForCompletion 是否等待所有命令完成
   */
  void Shutdown(bool waitForCompletion = true);
  /**
   * @brief 检查系统是否已初始化
   * @return bool 是否已初始化
   */
  bool IsInitialized() const;

  // ==================== 命令注册/创建接口（CommandRegistry核心方法再封装）
  // ====================
  template <typename T>
  bool RegisterCommandType();
  template <typename... Types>
  void RegisterCommandTypes();
  template <typename T>
  bool IsCommandTypeRegistered() const;
  template <typename T>
  CommandHandle CreateCommand();
  bool HasCommand(const CommandHandle &handle) const;
  bool ReleaseCommand(const CommandHandle &handle);

  // ====================
  // 命令执行/重做/撤销接口（CommandExecutor核心方法再封装）
  // ====================
  /**
   * @brief 执行命令（同步，自动处理Undo/Redo栈）
   * @param handle 命令句柄
   * @return CommandResult 执行结果
   */
  CommandResult Execute(CommandHandle handle);
  /**
   * @brief 异步提交命令
   * @param handle 命令句柄
   * @param priority 执行优先级
   * @return CommandResult 提交结果
   */
  CommandResult Submit(CommandHandle handle,
                       CommandPriority priority = CommandPriority::NORMAL);
  /**
   * @brief 创建并执行命令
   * @tparam T 命令类型
   * @return CommandResult 执行结果
   */
  template <typename T>
  CommandResult ExecuteNew();
  /**
   * @brief 创建并异步提交命令
   * @tparam T 命令类型
   * @param priority 执行优先级
   * @return CommandResult 提交结果
   */
  template <typename T>
  CommandResult SubmitNew(CommandPriority priority = CommandPriority::NORMAL);
  /**
   * @brief 执行撤销操作
   * @return CommandResult 撤销执行结果
   */
  CommandResult Undo();
  /**
   * @brief 执行异步撤销操作
   * @return bool 提交成功与否
   */
  bool UndoSubmit();
  /**
   * @brief 执行重做操作
   * @return CommandResult 重做执行结果
   */
  CommandResult Redo();
  /**
   * @brief 执行异步撤销操作
   * @return bool 提交成功与否
   */
  bool RedoSubmit();
  /**
   * @brief 检查是否可以撤销
   * @return bool 是否可以撤销
   */
  bool CanUndo() const;
  /**
   * @brief 检查是否可以重做
   * @return bool 是否可以重做
   */
  bool CanRedo() const;
  /**
   * @brief 获取撤销栈大小
   * @return size_t 撤销栈中命令数量
   */
  size_t GetUndoStackSize() const;
  /**
   * @brief 获取重做栈大小
   * @return size_t 重做栈中命令数量
   */
  size_t GetRedoStackSize() const;
  /**
   * @brief 清空Undo/Redo栈
   */
  void ClearStacks();
  /**
   * @brief 设置最大栈大小
   * @param maxSize 最大栈大小
   */
  void SetMaxStackSize(size_t maxSize);

  // ==================== 底层组件访问接口 ====================
  /**
   * @brief 获取命令执行器
   * @return CommandExecutor& 执行器引用
   */
  CommandExecutor &GetExecutor();
  /**
   * @brief 获取命令注册表
   * @return CommandRegistry& 注册表引用
   */
  CommandRegistry &GetRegistry();
  /**
   * @brief 获取撤销栈
   * @return CommandUndoStack& 撤销栈引用
   */
  CommandUndoStack &GetUndoStack();
  /**
   * @brief 获取重做栈
   * @return CommandRedoStack& 重做栈引用
   */
  CommandRedoStack &GetRedoStack();

 private:
  void OnCommandCompleted(CommandCompletedEvent &event);

  CommandSystem();
  ~CommandSystem();
  // 禁止拷贝
  CommandSystem(const CommandSystem &) = delete;
  CommandSystem &operator=(const CommandSystem &) = delete;

  // 成员变量
  std::unique_ptr<CommandRegistry> m_Registry;
  std::unique_ptr<CommandExecutor> m_Executor;
  std::unique_ptr<CommandUndoStack> m_UndoStack;
  std::unique_ptr<CommandRedoStack> m_RedoStack;

  std::atomic<bool> m_initialized{false};

  Logger m_Logger;
  SubscriptionGroup m_EventSubscriptions;  // 事件订阅
};

// ==================== 模板方法实现 ====================

template <typename T>
bool CommandSystem::RegisterCommandType() {
  return m_Registry->RegisterCommandType<T>();
}

template <typename... Types>
void CommandSystem::RegisterCommandTypes() {
  (RegisterCommandType<Types>(), ...);
}

template <typename T>
bool CommandSystem::IsCommandTypeRegistered() const {
  return m_Registry->IsCommandTypeRegistered<T>();
}

template <typename T>
inline CommandHandle CommandSystem::CreateCommand() {
  return m_Registry->CreateCommand<T>();
}

template <typename T>
CommandResult CommandSystem::ExecuteNew() {
  CommandHandle handle = m_Registry->CreateCommand<T>();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to create command");
  }
  return Execute(handle);
}
template <typename T>
CommandResult CommandSystem::SubmitNew(CommandPriority priority) {
  CommandHandle handle = m_Registry->CreateCommand<T>();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to create command");
  }
  return Submit(handle, priority);
}
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
