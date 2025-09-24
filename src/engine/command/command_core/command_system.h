#ifndef MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
#define MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM


#include "command_core/command_event.h"
#include "command_core/command.h"
#include "command_executor/command_execution_context.h"
#include "command_executor/command_executor.h"
#include "command_core/command_registry.h"
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
  };
  /**
   * @brief 初始化命令系统
   * @param initParams 初始化参数
   */
  void Initialize(const CommandSystemInitParams &initParams = {});
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

  // ==================== 命令注册接口 ====================
  /**
   * @brief 注册命令类型
   * @tparam T 命令类型
   * @return bool 注册是否成功
   */
  template<typename T> bool RegisterCommandType();
  /**
   * @brief 批量注册命令类型
   * @tparam Types 命令类型列表
   */
  template<typename... Types> void RegisterCommandTypes();
  /**
   * @brief 检查命令类型是否已注册
   * @tparam T 命令类型
   * @return bool 是否已注册
   */
  template<typename T> bool IsCommandTypeRegistered() const;

  // ==================== 命令执行接口 ====================
  /**
   * @brief 执行命令（同步，自动处理Undo/Redo栈）
   * @param handle 命令句柄
   * @param contextName 上下文名称（可选）
   * @return CommandResult 执行结果
   */
  CommandResult Execute(CommandHandle handle, const std::string &contextName = "Default");
  /**
   * @brief 异步提交命令
   * @param handle 命令句柄
   * @param contextName 上下文名称（可选）
   * @param priority 执行优先级
   * @return CommandResult 提交结果
   */
  CommandResult Submit(CommandHandle handle,
                       const std::string &contextName = "Default",
                       CommandPriority priority = CommandPriority::NORMAL);
  /**
   * @brief 创建并执行命令
   * @tparam T 命令类型
   * @param contextName 上下文名称
   * @return CommandResult 执行结果
   */
  template<typename T> CommandResult ExecuteNew(const std::string &contextName = "Default");
  /**
   * @brief 创建并异步提交命令
   * @tparam T 命令类型
   * @param contextName 上下文名称
   * @param priority 执行优先级
   * @return CommandResult 提交结果
   */
  template<typename T>
  CommandResult SubmitNew(const std::string &contextName = "Default",
                          CommandPriority priority = CommandPriority::NORMAL);

  // ==================== 上下文管理接口 ====================
  /**
   * @brief 创建或获取执行上下文
   * @param name 上下文名称
   * @param contextFlags 上下文标志
   * @return CommandExecutionContext* 上下文指针
   */
  CommandExecutionContext *GetOrCreateContext(const std::string &name,
                                              CommandContextFlags contextFlags = CONTEXT_NONE);
  /**
   * @brief 获取执行上下文
   * @param name 上下文名称
   * @return CommandExecutionContext* 上下文指针，不存在返回nullptr
   */
  CommandExecutionContext *GetContext(const std::string &name) const;
  /**
   * @brief 移除执行上下文
   * @param name 上下文名称
   */
  void RemoveContext(const std::string &name);

  // ==================== Undo/Redo 接口 ====================
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
  std::unique_ptr<CommandExecutor> m_executor;
  std::unique_ptr<CommandUndoStack> m_undoStack;
  std::unique_ptr<CommandRedoStack> m_redoStack;
  std::unordered_map<std::string, ExecutionContextPtr> m_contexts;

  std::atomic<bool> m_initialized{false};

  Logger m_Logger;
  SubscriptionGroup m_EventSubscriptions;  // 事件订阅
};

// ==================== 模板方法实现 ====================

template<typename T> bool CommandSystem::RegisterCommandType()
{
  return CommandRegistry::Get().RegisterCommandType<T>();
}

template<typename... Types> void CommandSystem::RegisterCommandTypes()
{
  (RegisterCommandType<Types>(), ...);
}

template<typename T> bool CommandSystem::IsCommandTypeRegistered() const
{
  return CommandRegistry::Get().IsCommandTypeRegistered<T>();
}

template<typename T> CommandResult CommandSystem::ExecuteNew(const std::string &contextName)
{
  CommandHandle handle = CommandRegistry::Get().CreateCommand<T>();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to create command");
  }
  return Execute(handle, contextName);
}
template<typename T>
CommandResult CommandSystem::SubmitNew(const std::string &contextName, CommandPriority priority)
{
  CommandHandle handle = CommandRegistry::Get().CreateCommand<T>();
  if (!handle.IsValid()) {
    return CommandResult::Failure("Failed to create command");
  }
  return Submit(handle, contextName, priority);
}
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
