#ifndef MITE_ENGINE_COMMAND_CORE_COMMAND
#define MITE_ENGINE_COMMAND_CORE_COMMAND

#include "command_type.h"

namespace mite {
/**
 * @brief 命令抽象基类（默认不可撤销，可撤销需要继承自CommandUndoable）
 *
 * 所有具体命令的基类，提供统一的命令接口和基础功能
 * 不允许直接创建Command对象，必须通过具体子类实现
 *
 * 子类必须重写的接口：
 * CommandResult Execute() override;        // 执行命令
 *
 * 必须调用的宏（省略GetName和GetCategory）
 * COMMAND_CLASS(type, category)
 *
 * 可选重写的接口
 * std::string GetDescription() override;   // 获取命令描述文本
 */
class Command {
 public:
  virtual ~Command() = default;

  // ==================== 核心执行接口 ====================
  /**
   * @brief 执行命令
   * @return CommandResult 命令执行结果
   */
  virtual CommandResult Execute() = 0;
  /**
   * @brief 撤销命令（不强制要求子类必须实现）
   * @return CommandResult 撤销执行结果
   */
  virtual CommandResult Undo(){};

  // ==================== 命令标识接口 ====================
  /**
   * @brief 获取命令名称（用于显示和调试）
   * @return const char* 命令名称
   */
  virtual std::string GetName() const = 0;
  /**
   * @brief 获取命令描述信息
   * @return std::string 命令描述
   */
  virtual std::string GetDescription() const
  {
    return "";
  }

  // ==================== 命令分类接口 ====================
  /**
   * @brief 获取命令类别（位掩码）
   * @return CommandCategory 命令类别
   */
  virtual CommandCategory GetCategory() const = 0;
  /**
   * @brief 检查命令是否属于指定类别
   * @param category 要检查的类别
   * @return bool 是否属于该类别
   */
  bool IsInCategory(CommandCategory category) const
  {
    return (GetCategory() & category) != 0;
  }

  // ==================== 优先级管理接口 ====================
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

  // ==================== 撤销重做接口 ====================
  /**
   * @brief 检查命令是否可撤销
   * @return bool 是否可撤销（默认不可撤销，可撤销的命令需要继承自CommandUndoable）
   */
  virtual bool CanUndo() const
  {
    return false;
  }
  /**
   * @brief 检查命令是否可重做
   * @return bool 是否可重做
   */
  virtual bool CanRedo() const
  {
    return CanUndo();
  }

 protected:
  CommandPriority m_priority;  // 命令优先级
  // 禁止拷贝和赋值
  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;
};

/**
 * @brief 可撤销的命令抽象基类
 *
 * 子类必须重写的接口：
 * CommandResult Execute() override;        // 执行命令
 * CommandResult Undo() override;           // 撤销命令
 *
 * 必须调用的宏（省略GetName和GetCategory）
 * COMMAND_CLASS(type, category)
 *
 * 可选重写的接口
 * std::string GetDescription() override;   // 获取命令描述文本
 */
class CommandUndoable : public Command {
 public:
  virtual ~CommandUndoable() = default;

  // 强制要求子类实现Undo方法
  virtual CommandResult Undo() = 0;

  // CanUndo标记为True
  virtual bool CanUndo() const
  {
    return true;
  }
};

// 命令智能指针类型
using CommandPtr = std::unique_ptr<Command>;

// Command派生类辅助宏，用于自动实现GetName和GetCategory
#define COMMAND_CLASS_TYPE(type) \
  virtual std::string GetName() const override \
  { \
    return #type; \
  }
#define COMMAND_CLASS_CATEGORY(category) \
  virtual CommandCategory GetCategory() const override \
  { \
    return category; \
  }
// 组合宏：一行搞定GetName和GetCategory的实现
#define COMMAND_CLASS(type, category) \
  COMMAND_CLASS_TYPE(type) \
  COMMAND_CLASS_CATEGORY(category)
}  // namespace mite

#endif  // MITE_ENGINE_COMMAND_CORE_COMMAND
