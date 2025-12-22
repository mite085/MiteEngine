#ifndef MITE_COMMAND_EVENTS
#define MITE_COMMAND_EVENTS

#include "command.h"
#include "command_executor/command_execution_context.h"
#include "subscription_group.h"

namespace mite {
/**
 * @brief 命令执行事件（不持有命令所有权）
 *
 * 当命令开始执行时触发，仅包含命令信息（不持有所有权）
 */
class CommandExecuteEvent : public Event {
 public:
  /**
   * @brief 命令执行事件构造函数
   * @param commandInfo 命令信息（不持有所有权）
   * @param context 执行上下文
   * @param priority 命令优先级
   */
  explicit CommandExecuteEvent(const Command *commandInfo,
                               CommandExecutionContext *context)
      : m_CommandInfo(commandInfo), m_Context(context) {}
  const Command *GetCommandInfo() const { return m_CommandInfo; }
  CommandExecutionContext *GetContext() const { return m_Context; }
  CommandPriority GetPriority() const { return m_CommandInfo->GetPriority(); }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override {
    return new CommandExecuteEvent(m_CommandInfo, m_Context);
  }

 private:
  const Command *m_CommandInfo;  // 仅提供信息，不持有所有权
  CommandExecutionContext *m_Context;
};
/**
 * @brief 命令完成事件
 *
 * 当命令执行完成时触发，包含执行结果和命令对象的所有权
 */
class CommandCompletedEvent : public Event {
 public:
  /**
   * @brief 命令完成事件构造函数（接管所有权）
   * @param result 执行结果（包含命令对象）
   */
  explicit CommandCompletedEvent(CommandResult result) : m_Result(result) {}
  /**
   * @brief 获取执行结果（不转移所有权）
   */
  const CommandResult &GetResult() const { return m_Result; }
  /**
   * @brief 检查是否包含命令对象
   */
  bool HasCommandHandle() const { return m_Result.HasCommandHandle(); }
  /**
   * @brief 获取命令句柄
   */
  const CommandHandle &GetCommandHandle() const {
    return m_Result.GetCommandHandle();
  }
  bool IsSuccess() const { return m_Result.success; }
  CommandExecutionState GetState() const { return m_Result.state; }
  const std::string &GetMessage() const { return m_Result.message; }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override { return new CommandCompletedEvent(m_Result); }

 private:
  CommandResult m_Result;  // 持有命令对象的所有权
};
/**
 * @brief 命令撤销事件
 */
class CommandUndoEvent : public Event {
 public:
  /**
   * @brief 命令撤销事件构造函数（接管所有权）
   * @param result 撤销操作结果（包含命令对象）
   */
  explicit CommandUndoEvent(CommandHandle handle) : m_Handle(handle) {}
  const CommandHandle &GetHandle() const { return m_Handle; }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override { return new CommandUndoEvent(m_Handle); }

 private:
  CommandHandle m_Handle;
};
/**
 * @brief 命令重做事件
 */
class CommandRedoEvent : public Event {
 public:
  /**
   * @brief 命令重做事件构造函数（接管所有权）
   * @param result 重做操作结果（包含命令对象）
   */
  explicit CommandRedoEvent(CommandHandle handle) : m_Handle(handle) {}
  const CommandHandle &GetHandle() const { return m_Handle; }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override { return new CommandRedoEvent(m_Handle); }

 private:
  CommandHandle m_Handle;
};
};  // namespace mite

#endif  // MITE_COMMAND_EVENTS
