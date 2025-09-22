#ifndef MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT
#define MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT

#include "command_core/command.h"

namespace mite {
/**
 * @brief 命令执行上下文
 *
 * 提供命令执行的环境信息和管理功能，包括上下文标志、执行状态跟踪等
 * 
 * 使用示例：
 * 
 * // 1: 创建特定上下文
 * auto* editorContext = new CommandExecutionContext(CONTEXT_EDITOR, "Editor");
 * editorContext->SetAllowedCategories(COMMAND_CATEGORY_EDITOR | COMMAND_CATEGORY_UI);// 设置只允许编辑器相关的命令
 * editorContext->SetForbiddenCategories(COMMAND_CATEGORY_SYSTEM);
 * editorContext->AddForbiddenCommandType(typeid(DeleteAllEntitiesCommand));// 禁止特定的危险命令
 * editorContext->AddCommandFilter([](const Command* cmd) { // 添加自定义过滤器  
 *     return !cmd->GetName().starts_with("Experimental");// 只允许在特定时间执行某些命令
 * });
 * 
 * // 2: 运行时上下文
 * auto* runtimeContext = new CommandExecutionContext(CONTEXT_RUNTIME, "Runtime");
 * runtimeContext->SetAllowedCategories(COMMAND_CATEGORY_ENTITY | COMMAND_CATEGORY_TRANSFORM);
 * runtimeContext->SetForbiddenCategories(COMMAND_CATEGORY_EDITOR);
 * 
 * // 3: 检查命令可用性
 * auto moveCmd = CommandFactory::Get().Create<MoveEntityCommand>();
 * if (editorContext->IsCommandAvailable(moveCmd.get())) {
 *     executor.SubmitCommand(std::move(moveCmd), editorContext);
 * }
 * // 4: 批量检查类型可用性
 * auto availableTypes = editorContext->GetAllowedCommandTypes();
 * for (const auto& type : availableTypes) {
 *     LOG_INFO("Available command: {}", CommandRegistry::Get().GetCommandTypeName(type));
 * }
*/
class CommandExecutionContext {
 public:
  // ==================== 类型定义 ====================
  using CommandFilter = std::function<bool(const Command *)>;
  using TypeFilter = std::function<bool(std::type_index)>;

  // ==================== 构造函数和析构函数 ====================
  /**
   * @brief 构造函数
   * @param contextFlags 上下文标志
   * @param name 上下文名称（用于调试）
   */
  explicit CommandExecutionContext(CommandContextFlags contextFlags = CONTEXT_NONE,
                                   const std::string &name = "");
  virtual ~CommandExecutionContext();

  // ==================== 上下文标志管理接口 ====================
  /**
   * @brief 获取上下文标志
   * @return uint32_t 上下文标志
   */
  uint32_t GetContextFlags() const;
  /**
   * @brief 设置上下文标志
   * @param flags 新的上下文标志
   */
  void SetContextFlags(uint32_t flags);
  /**
   * @brief 添加上下文标志
   * @param flags 要添加的标志
   */
  void AddContextFlags(uint32_t flags);
  /**
   * @brief 移除上下文标志
   * @param flags 要移除的标志
   */
  void RemoveContextFlags(uint32_t flags);
  /**
   * @brief 检查是否包含指定上下文标志
   * @param flags 要检查的标志
   * @return bool 是否包含
   */
  bool HasContextFlags(uint32_t flags) const;

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
   * @brief 检查命令是否在当前上下文中可用
   * @param command 要检查的命令
   * @return bool 是否可用
   */
  virtual bool IsCommandAvailable(Command *command) const;
  /**
   * @brief 检查命令类型是否在当前上下文中可用
   * @param typeIndex 命令类型索引
   * @return bool 是否可用
   */
  virtual bool IsCommandTypeAvailable(std::type_index typeIndex) const;

  // ==================== 过滤器管理接口实现 ====================
  /**
   * @brief 添加命令过滤器
   * @param filter 命令过滤函数
   */
  void AddCommandFilter(CommandFilter filter);
  /**
   * @brief 添加类型过滤器
   * @param filter 类型过滤函数
   */
  void AddTypeFilter(TypeFilter filter);
  /**
   * @brief 清除所有过滤器
   */
  void ClearFilters();
  /**
   * @brief 设置允许的命令类别
   * @param allowedCategories 允许的类别掩码
   */
  void SetAllowedCategories(CommandCategory allowedCategories);
  /**
   * @brief 设置禁止的命令类别
   * @param forbiddenCategories 禁止的类别掩码
   */
  void SetForbiddenCategories(CommandCategory forbiddenCategories);
  /**
   * @brief 添加允许的特定命令类型
   * @param typeIndex 允许的命令类型
   */
  void AddAllowedCommandType(std::type_index typeIndex);
  /**
   * @brief 添加禁止的特定命令类型
   * @param typeIndex 禁止的命令类型
   */
  void AddForbiddenCommandType(std::type_index typeIndex);
  /**
   * @brief 设置最小命令优先级
   * @param minPriority 最小优先级
   */
  void SetMinCommandPriority(CommandPriority minPriority);
  /**
   * @brief 设置最大命令优先级
   * @param maxPriority 最大优先级
   */
  void SetMaxCommandPriority(CommandPriority maxPriority);

  // ==================== 命令执行跟踪接口 ====================
  /**
   * @brief 记录命令执行开始
   * @param command 执行的命令
   */
  void RecordCommandExecutionStart(Command *command);
  /**
   * @brief 记录命令执行完成
   * @param command 完成的命令
   * @param result 执行结果
   */
  void RecordCommandExecutionComplete(Command *command, const CommandResult &result);
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

  // ==================== 上下文验证工具接口 ====================
  /**
   * @brief 获取所有允许的命令类型
   * @return std::vector<std::type_index> 允许的类型列表
   */
  std::vector<std::type_index> GetAllowedCommandTypes() const;
  /**
   * @brief 检查命令是否符合优先级要求
   * @param command 要检查的命令
   * @return bool 是否符合优先级要求
   */
  bool CheckCommandPriority(const Command *command) const;
  /**
   * @brief 检查命令类别是否符合要求
   * @param command 要检查的命令
   * @return bool 是否符合类别要求
   */
  bool CheckCommandCategory(const Command *command) const;

 protected:
  // ==================== 受保护的成员变量 ====================

  uint32_t m_contextFlags;  // 上下文标志
  std::string m_name;       // 上下文名称
  bool m_isActive;          // 是否处于活动状态

  // 执行统计
  size_t m_executingCount;                            // 正在执行的命令数量
  size_t m_completedCount;                            // 已完成的命令数量
  size_t m_succeededCount;                            // 成功的命令数量
  size_t m_failedCount;                               // 失败的命令数量
  std::unordered_set<Command *> m_ExecutingCommands;  // 当前正在执行的命令

  // 过滤器和限制条件
  std::vector<CommandFilter> m_commandFilters;
  std::vector<TypeFilter> m_typeFilters;
  std::unordered_set<std::type_index> m_allowedCommandTypes;
  std::unordered_set<std::type_index> m_forbiddenCommandTypes;
  CommandCategory m_allowedCategories{0xFFFFFFFFFFFFFFFF};  // 默认允许所有
  CommandCategory m_forbiddenCategories{0};                 // 默认不禁止任何
  CommandPriority m_minPriority{CommandPriority::LOW};
  CommandPriority m_maxPriority{CommandPriority::CRITICAL};
};

// 执行上下文智能指针
using ExecutionContextPtr = std::unique_ptr<CommandExecutionContext>;
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_EXECUTOR_COMMAND_EXECUTION_CONTEXT
