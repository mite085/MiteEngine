#ifndef MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
#define MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM


#include "command_core/command_event.h"
#include "command_core/command.h"
#include "command_executor/command_execution_context.h"
#include "command_executor/command_executor.h"
#include "command_core/command_factory.h"
#include "command_core/command_registry.h"
#include "command_executor/command_redo_stack.h"
#include "command_executor/command_undo_stack.h"

namespace mite {

/**
 * @brief 命令系统 - 统一管理所有命令相关功能
 *
 * 提供简化的接口，外部只需与CommandSystem交互即可完成绝大多数命令操作
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

  // ==================== 命令执行接口（简化版）====================
  /**
   * @brief 执行命令（自动处理Undo/Redo栈）
   * @param command 要执行的命令
   * @param contextName 上下文名称（可选）
   * @return CommandResult 执行结果
   */
  CommandResult Execute(CommandPtr command, const std::string &contextName = "Default");
  /**
   * @brief 异步提交命令
   * @param command 要执行的命令
   * @param contextName 上下文名称（可选）
   * @param priority 执行优先级
   * @return bool 提交是否成功
   */
  CommandResult Submit(CommandPtr command,
                       const std::string &contextName = "Default",
                       BS::priority_t priority = BS::pr::normal);
  /**
   * @brief 创建并执行命令
   * @tparam T 命令类型
   * @param args 命令参数
   * @param contextName 上下文名称
   * @return CommandResult 执行结果
   */
  template<typename T, typename... Args>
  CommandResult ExecuteNew(Args &&...args, const std::string &contextName = "Default");

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
   * @brief 执行重做操作
   * @return CommandResult 重做执行结果
   */
  CommandResult Redo();
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

  // ==================== 底层组件访问接口（高级用法）====================
  /**
   * @brief 获取命令执行器
   * @return CommandExecutor& 执行器引用
   */
  CommandExecutor &GetExecutor();
  /**
   * @brief 获取命令工厂
   * @return CommandFactory& 工厂引用
   */
  CommandFactory &GetFactory();
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
  return GetRegistry().RegisterCommandType<T>();
}

template<typename... Types> void CommandSystem::RegisterCommandTypes()
{
  (RegisterCommandType<Types>(), ...);
}

template<typename T> inline bool CommandSystem::IsCommandTypeRegistered() const
{
  return GetRegistry().IsCommandTypeRegistered<T>();
}

template<typename T, typename... Args>
CommandResult CommandSystem::ExecuteNew(Args &&...args, const std::string &contextName)
{
  auto command = GetFactory().Create<T>();
  if (!command) {
    return CommandResult::Failure("Failed to create command");
  }
  return Execute(std::move(command), contextName);
}

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_SYSTEM_COMMAND_SYSTEM
