#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND
#define MITE_ENGINE_COMMAND_CORE_COMMAND

#include "command_type.h"
#include "headers/headers.h"

namespace mite {

/**
 * @brief 命令抽象基类
 *
 * 所有具体命令的基类，提供统一的命令接口和基础功能
 * 不允许直接创建Command对象，必须通过具体子类实现
 */
class Command {
 public:
  virtual ~Command() = default;

  /**
   * @brief 执行命令
   * @return CommandResult 命令执行结果
   */
  virtual CommandResult Execute() = 0;

  /**
   * @brief 撤销命令
   * @return CommandResult 撤销执行结果
   */
  virtual CommandResult Undo() = 0;

  /**
   * @brief 获取命令唯一标识符
   * @return uint64_t 命令ID
   */
  virtual uint64_t GetId() const = 0;

  /**
   * @brief 获取命令名称（用于显示和调试）
   * @return const char* 命令名称
   */
  virtual const char *GetName() const = 0;

  /**
   * @brief 获取命令类别（位掩码）
   * @return CommandCategory 命令类别
   */
  virtual CommandCategory GetCategory() const = 0;

  /**
   * @brief 获取命令执行状态
   * @return CommandExecutionState 执行状态
   */
  CommandExecutionState GetState() const
  {
    return m_state;
  }

  /**
   * @brief 获取命令优先级
   * @return CommandPriority 命令优先级
   */
  CommandPriority GetPriority() const
  {
    return m_priority;
  }

  /**
   * @brief 设置命令优先级
   * @param priority 新的优先级
   */
  void SetPriority(CommandPriority priority)
  {
    m_priority = priority;
  }

  /**
   * @brief 检查命令是否可撤销
   * @return bool 是否可撤销
   */
  virtual bool CanUndo() const
  {
    return true;
  }

  /**
   * @brief 检查命令是否可合并
   * @param other 要合并的命令
   * @return bool 是否可合并
   */
  virtual bool CanMergeWith(const Command *other) const
  {
    return false;
  }

  /**
   * @brief 合并命令
   * @param other 要合并的命令
   * @return bool 合并是否成功
   */
  virtual bool MergeWith(const Command *other)
  {
    return false;
  }

  /**
   * @brief 获取命令合并策略
   * @return CommandMergePolicy 合并策略
   */
  virtual CommandMergePolicy GetMergePolicy() const
  {
    return CommandMergePolicy::NONE;
  }

 protected:
  /**
   * @brief 受保护的构造函数，确保只能通过子类创建
   * @param priority 命令优先级
   */
  explicit Command(CommandPriority priority = CommandPriority::NORMAL)
      : m_priority(priority), m_state(CommandExecutionState::PENDING)
  {
  }

  /**
   * @brief 设置命令执行状态（供子类使用）
   * @param state 新的执行状态
   */
  void SetState(CommandExecutionState state)
  {
    m_state = state;
  }

 private:
  CommandPriority m_priority;     // 命令优先级
  CommandExecutionState m_state;  // 命令执行状态

  // 禁止拷贝和赋值
  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;
};

// 命令智能指针类型
using CommandPtr = std::unique_ptr<Command>;

}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND
