#ifndef MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT
#define MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT

#include "command_core/command.h"
#include "command_core/command_registry.h"

namespace mite {
/**
 * @brief 命令执行上下文
 *
 * 提供命令执行的环境信息和管理功能，包括上下文标志、执行状态跟踪等
 * }
 */
class CommandExecutionContext {
 public:
  // ==================== 类型定义 ====================
  using CommandFilter = std::function<bool(const CommandHandle &)>;
  using TypeFilter = std::function<bool(std::type_index)>;

  // ==================== 构造函数和析构函数 ====================
  /**
   * @brief 构造函数
   * @param contextFlags 上下文标志
   * @param name 上下文名称（用于调试）
   */
  explicit CommandExecutionContext(
      CommandContextFlags contextFlags = CONTEXT_NONE,
      const std::string &name = "");
  virtual ~CommandExecutionContext();

  // ==================== 上下文标志管理接口 ====================
  /**
   * @brief 获取上下文标志
   * @return uint32_t 上下文标志
   */
  CommandContextFlags GetContextFlags() const;
  /**
   * @brief 设置上下文标志
   * @param flags 新的上下文标志
   */
  void SetContextFlags(CommandContextFlags flags);
  /**
   * @brief 添加上下文标志
   * @param flags 要添加的标志
   */
  void AddContextFlags(CommandContextFlags flags);
  /**
   * @brief 移除上下文标志
   * @param flags 要移除的标志
   */
  void RemoveContextFlags(CommandContextFlags flags);
  /**
   * @brief 检查是否包含指定上下文标志
   * @param flags 要检查的标志
   * @return bool 是否包含
   */
  bool HasContextFlags(CommandContextFlags flags) const;

  // ==================== 上下文状态管理接口 ====================
  /**
   * @brief 检查上下文是否处于活动状态
   * @return bool 是否活动
   */
  bool IsActive() const;
  /**
   * @brief 激活上下文
   */
  void Activate();
  /**
   * @brief 停用上下文
   */
  void Deactivate();
  /**
   * @brief 获取上下文名称
   * @return const std::string& 上下文名称
   */
  const std::string &GetName() const;

  // ==================== 命令可用性检查接口 ====================
  /**
   * @brief 检查命令是否在当前上下文中可用（基于句柄）
   * @param handle 命令句柄
   * @return bool 是否可用
   */
  virtual bool IsCommandAvailable(const CommandRegistry &registry,
                                  const CommandHandle &handle) const;
  /**
   * @brief 检查命令类型是否在当前上下文中可用
   * @param typeIndex 命令类型索引
   * @return bool 是否可用
   */
  virtual bool IsCommandTypeAvailable(const CommandRegistry &registry,
                                      std::type_index typeIndex) const;

  // ==================== 命令执行跟踪接口 ====================
  /**
   * @brief 记录命令执行开始（基于句柄）
   * @param handle 执行的命令句柄
   */
  void RecordCommandExecutionStart(const CommandHandle &handle);
  /**
   * @brief 记录命令执行完成（基于句柄）
   * @param handle 完成的命令句柄
   * @param result 执行结果
   */
  void RecordCommandExecutionComplete(const CommandHandle &handle,
                                      const CommandResult &result);
  /**
   * @brief 获取当前正在执行的命令数量
   * @return size_t 执行中的命令数量
   */
  size_t GetExecutingCommandCount() const;
  /**
   * @brief 获取已完成的命令数量
   * @return size_t 完成的命令数量
   */
  size_t GetCompletedCommandCount() const;
  /**
   * @brief 获取成功的命令数量
   * @return size_t 成功的命令数量
   */
  size_t GetSucceededCommandCount() const;
  /**
   * @brief 获取失败的命令数量
   * @return size_t 失败的命令数量
   */
  size_t GetFailedCommandCount() const;

  // ==================== 句柄管理接口 ====================
  /**
   * @brief 获取当前正在执行的命令句柄列表
   * @return std::vector<CommandHandle> 执行中的命令句柄
   */
  std::vector<CommandHandle> GetExecutingCommandHandles() const;
  /**
   * @brief 检查句柄是否正在执行
   * @param handle 命令句柄
   * @return bool 是否正在执行
   */
  bool IsCommandExecuting(const CommandHandle &handle) const;
  /**
   * @brief 清除所有执行记录
   */
  void ClearExecutionRecords();

 protected:
  // ==================== 受保护的成员变量 ====================
  Logger m_Logger;
  CommandContextFlags m_contextFlags;  // 上下文标志
  std::string m_name;                  // 上下文名称
  bool m_isActive;                     // 是否处于活动状态

  // 执行统计
  mutable std::shared_mutex m_ExecutionMutex;
  size_t m_ExecutingCount;  // 正在执行的命令数量
  size_t m_CompletedCount;  // 已完成的命令数量
  size_t m_SucceededCount;  // 成功的命令数量
  size_t m_FailedCount;     // 失败的命令数量
  std::unordered_set<CommandHandle> m_ExecutingCommands;  // 当前正在执行的命令
};

// 执行上下文智能指针
using ExecutionContextPtr = std::unique_ptr<CommandExecutionContext>;
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT
