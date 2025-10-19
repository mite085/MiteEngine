#ifndef MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTOR
#define MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTOR

#include "command_core/command.h"
#include "command_core/command_registry.h"
#include "command_execution_context.h"

namespace mite {

/**
 * @brief 命令执行器
 *
 * 负责命令的调度和执行，使用CommandHandle作为命令的唯一标识
 * 支持同步和异步执行模式，正确处理命令所有权转移
 * 
 * 使用示例：
 *
 * // 1. 基础使用
 * CommandExecutor executor;
 * executor.Start();
 *
 * // 2. 创建执行上下文（目前使用默认上下文即可）
 * auto* context = executor.CreateDefaultExecutionContext(CONTEXT_EDITOR, "Editor");
 *
 * // 3. 异步提交命令（默认优先级）
 * auto moveCmd = CommandRegister::Get().Create<MoveEntityCommand>();
 * executor.SubmitCommand(std::move(moveCmd), context); // 转移所有权
 *
 * // 4. 异步提交命令（指定优先级）
 * auto criticalCmd = CommandRegister::Get().Create<DeleteEntityCommand>();
 * executor.SubmitCommand(std::move(criticalCmd), context, BS::pr::high); // 转移所有权
 *
 * // 5. 同步执行命令
 * auto saveCmd = CommandRegister::Get().Create<SaveSceneCommand>();
 * auto result = executor.ExecuteCommand(std::move(saveCmd), context); // 转移所有权
 *
 * // 6. 批量提交命令
 * std::vector<CommandPtr> commands;
 * commands.push_back(CommandRegister::Get().Create<RotateEntityCommand>());
 * commands.push_back(CommandRegister::Get().Create<ScaleEntityCommand>());
 * executor.SubmitCommands(std::move(commands), context, BS::pr::normal);  // 转移所有权
 *
 * // 7. 执行控制
 * executor.Pause();  // 暂停新任务提交
 * executor.Resume(); // 恢复任务提交
 *
 * // 8. 执行统计
 * size_t executing = executor.GetExecutingCommandCount();
 * size_t completed = executor.GetCompletedCommandCount();
 *
 * // 9. 停止执行器（等待所有任务完成）
 * executor.Stop(true);
 *
 * 注意事项：
 * - 执行器需要先调用Start()才能提交命令
 * - 同步执行会阻塞当前线程直到命令完成
 * - 异步执行通过线程池调度，支持优先级
 * - 执行上下文用于管理命令的执行环境
 */
class CommandExecutor {
 public:
  // ==================== 构造函数和析构函数 ====================
  CommandExecutor(CommandRegistry &registry);
  virtual ~CommandExecutor();

  // ==================== 执行器状态管理接口 ====================
  /**
   * @brief 启动执行器
   */
  void Start();
  /**
   * @brief 停止执行器
   * @param waitForCompletion 是否等待所有命令完成
   */
  void Stop(bool waitForCompletion = true);
  /**
   * @brief 检查执行器是否正在运行
   * @return bool 是否运行中
   */
  bool IsRunning() const;

  // ==================== 命令提交接口 ====================
  /**
   * @brief 提交命令执行（同步，使用句柄）
   * @param handle 命令句柄
   * @param context 执行上下文（可选，若为nullptr则使用默认上下文）
   * @return CommandResult 执行结果
   */
  CommandResult ExecuteCommand(CommandHandle handle, CommandExecutionContext *context = nullptr);
  /**
   * @brief 提交命令执行（异步，使用句柄）
   * @param handle 命令句柄
   * @param context 执行上下文（可选，若为nullptr则使用默认上下文）
   * @param priority 执行优先级
   * @return bool 提交是否成功
   */
  bool SubmitCommandAsync(CommandHandle handle,
                          CommandExecutionContext *context = nullptr,
                          CommandPriority priority = CommandPriority::NORMAL);
  /**
   * @brief 批量提交命令（异步，使用句柄列表）
   * @param handles 命令句柄列表
   * @param context 执行上下文（可选，若为nullptr则使用默认上下文）
   * @param priority 执行优先级
   * @return size_t 成功提交的命令数量
   */
  size_t SubmitCommands(const std::vector<CommandHandle> &handles,
                        CommandExecutionContext *context = nullptr,
                        CommandPriority priority = CommandPriority::NORMAL);

  // ==================== 执行上下文管理接口 ====================
  /**
   * @brief 设置默认执行上下文
   * @param context 执行上下文
   */
  void SetDefaultExecutionContext(ExecutionContextPtr context);
  /**
   * @brief 获取默认执行上下文
   * @return CommandExecutionContext* 默认执行上下文指针
   */
  CommandExecutionContext *GetDefaultExecutionContext() const;
  /**
   * @brief 创建并设置默认执行上下文
   * @param contextFlags 上下文标志
   * @param name 上下文名称
   * @return CommandExecutionContext* 新创建的上下文指针
   */
  CommandExecutionContext *CreateDefaultExecutionContext(
      CommandContextFlags contextFlags = CONTEXT_NONE, const std::string &name = "Default");

  // ==================== 执行统计接口 ====================
  /**
   * @brief 获取待执行命令数量（BS线程池内部管理队列，无法直接获取）
   * @return size_t 待执行命令数量
   */
  //size_t GetPendingCommandCount() const;

  /**
   * @brief 获取正在执行的命令数量
   * @return size_t 执行中的命令数量
   */
  size_t GetExecutingCommandCount() const;
  /**
   * @brief 获取已完成的命令数量
   * @return size_t 完成的命令数量
   */
  size_t GetCompletedCommandCount() const;
  /**
   * @brief 获取执行器启动以来的总命令数量
   * @return size_t 总命令数量
   */
  size_t GetTotalCommandCount() const;

  // ==================== 执行控制接口 ====================
  /**
   * @brief 暂停命令执行
   */
  void Pause();
  /**
   * @brief 恢复命令执行
   */
  void Resume();
  /**
   * @brief 检查是否处于暂停状态
   * @return bool 是否暂停
   */
  bool IsPaused() const;
  /**
   * @brief 清空待执行命令队列
   * @return size_t 被清空的命令数量
   */
  size_t ClearPendingCommands();

 private:
  // ==================== 内部结构 ====================
  struct CommandTask {
    CommandHandle handle;
    CommandExecutionContext *context;
    std::type_index expectedType;
    CommandTask(CommandHandle h, CommandExecutionContext *ctx, std::type_index type)
        : handle(h), context(ctx), expectedType(type)
    {
    }

    // 拷贝构造函数
    CommandTask(const CommandTask &other) = default;
    // 拷贝赋值运算符
    CommandTask &operator=(const CommandTask &other) = default;
    // 移动构造函数
    CommandTask(CommandTask &&other) noexcept
        : handle(other.handle), context(other.context), expectedType(other.expectedType)
    {
    }
    // 移动赋值运算符
    CommandTask &operator=(CommandTask &&other) noexcept
    {
      if (this != &other) {
        handle = other.handle;
        context = other.context;
        expectedType = other.expectedType;
      }
      return *this;
    }
  };
  // 命令执行函数
  CommandResult ExecuteSingleCommand(CommandTask task);

  // ==================== 成员变量 ====================
  Logger m_Logger;
  mutable std::mutex m_Mutex;
  ExecutionContextPtr m_DefaultContext;
  CommandRegistry &m_Registry;

  // ExecuteSingleCommand可能在子线程内运行，需要确保线程安全
  std::atomic<bool> m_IsRunning{false};
  std::atomic<bool> m_IsPaused{false};
  std::atomic<size_t> m_TotalCommands{0};
  std::atomic<size_t> m_CompletedCommands{0};
  std::atomic<size_t> m_ExecutingCommands{0};

  // 使用BS线程池的任务future存储，用于等待任务完成
  std::vector<std::future<void>> m_TaskFutures;
};

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTOR
