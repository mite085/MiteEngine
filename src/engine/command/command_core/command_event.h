#ifndef MITE_COMMAND_EVENTS
#define MITE_COMMAND_EVENTS

#include "subscription_group.h"
#include "command.h"
#include "command_executor/command_execution_context.h"

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
                               CommandExecutionContext *context,
                               CommandPriority priority = CommandPriority::NORMAL)
      : m_CommandInfo(commandInfo), m_Context(context), m_Priority(priority)
  {
  }
  const Command *GetCommandInfo() const
  {
    return m_CommandInfo;
  }
  CommandExecutionContext *GetContext() const
  {
    return m_Context;
  }
  CommandPriority GetPriority() const
  {
    return m_Priority;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    return new CommandExecuteEvent(m_CommandInfo, m_Context, m_Priority);
  }

 private:
  const Command *m_CommandInfo;  // 仅提供信息，不持有所有权
  CommandExecutionContext *m_Context;
  CommandPriority m_Priority;
};
/**
 * @brief 命令完成事件（持有命令所有权）
 *
 * 当命令执行完成时触发，包含执行结果和命令对象的所有权
 */
class CommandCompletedEvent : public Event {
 public:
  /**
   * @brief 命令完成事件构造函数（接管所有权）
   * @param result 执行结果（包含命令对象）
   */
  explicit CommandCompletedEvent(CommandResult result) : m_Result(std::move(result)) {}
  /**
   * @brief 获取执行结果（不转移所有权）
   */
  const CommandResult &GetResult() const
  {
    return m_Result;
  }
  /**
   * @brief 检查是否包含命令对象
   */
  bool HasCommand() const
  {
    return m_Result.HasCommand();
  }
  /**
   * @brief 获取命令信息（不转移所有权）
   */
  const Command *GetCommandInfo() const
  {
    return m_Result.GetCommand();
  }
  /**
   * @brief 释放命令对象所有权
   */
  CommandPtr ReleaseCommand()
  {
    return m_Result.ReleaseCommand();
  }
  bool IsSuccess() const
  {
    return m_Result.success;
  }
  CommandExecutionState GetState() const
  {
    return m_Result.state;
  }
  const std::string &GetMessage() const
  {
    return m_Result.message;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    // 克隆时创建新的CommandResult，但不复制命令对象
    CommandResult clonedResult(m_Result.success, m_Result.state, m_Result.message);
    return new CommandCompletedEvent(std::move(clonedResult));
  }

 private:
  CommandResult m_Result;  // 持有命令对象的所有权
};
/**
 * @brief 命令撤销事件（持有命令所有权）
 */
class CommandUndoEvent : public Event {
 public:
  /**
   * @brief 命令撤销事件构造函数（接管所有权）
   * @param result 撤销操作结果（包含命令对象）
   */
  explicit CommandUndoEvent(CommandResult result) : m_Result(std::move(result)) {}
  const CommandResult &GetResult() const
  {
    return m_Result;
  }
  bool HasCommand() const
  {
    return m_Result.HasCommand();
  }
  const Command *GetCommandInfo() const
  {
    return m_Result.GetCommand();
  }
  CommandPtr ReleaseCommand()
  {
    return m_Result.ReleaseCommand();
  }
  bool IsSuccess() const
  {
    return m_Result.success;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    CommandResult clonedResult(m_Result.success, m_Result.state, m_Result.message);
    return new CommandUndoEvent(std::move(clonedResult));
  }

 private:
  CommandResult m_Result;
};
/**
 * @brief 命令重做事件（持有命令所有权）
 */
class CommandRedoEvent : public Event {
 public:
  /**
   * @brief 命令重做事件构造函数（接管所有权）
   * @param result 重做操作结果（包含命令对象）
   */
  explicit CommandRedoEvent(CommandResult result) : m_Result(std::move(result)) {}
  const CommandResult &GetResult() const
  {
    return m_Result;
  }
  bool HasCommand() const
  {
    return m_Result.HasCommand();
  }
  const Command *GetCommandInfo() const
  {
    return m_Result.GetCommand();
  }
  CommandPtr ReleaseCommand()
  {
    return m_Result.ReleaseCommand();
  }
  bool IsSuccess() const
  {
    return m_Result.success;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    CommandResult clonedResult(m_Result.success, m_Result.state, m_Result.message);
    return new CommandRedoEvent(std::move(clonedResult));
  }

 private:
  CommandResult m_Result;
};
/**
 * @brief 命令状态变更事件（不持有命令所有权）
 */
class CommandStateChangedEvent : public Event {
 public:
  /**
   * @brief 命令状态变更事件构造函数
   * @param commandInfo 状态变更的命令信息（不持有所有权）
   * @param oldState 旧状态
   * @param newState 新状态
   */
  explicit CommandStateChangedEvent(const Command *commandInfo,
                                    CommandExecutionState oldState,
                                    CommandExecutionState newState)
      : m_CommandInfo(commandInfo), m_OldState(oldState), m_NewState(newState)
  {
  }
  const Command *GetCommandInfo() const
  {
    return m_CommandInfo;
  }
  CommandExecutionState GetOldState() const
  {
    return m_OldState;
  }
  CommandExecutionState GetNewState() const
  {
    return m_NewState;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    return new CommandStateChangedEvent(m_CommandInfo, m_OldState, m_NewState);
  }

 private:
  const Command *m_CommandInfo;  // 仅提供信息，不持有所有权
  CommandExecutionState m_OldState;
  CommandExecutionState m_NewState;
};
/**
 * @brief 命令队列状态事件
 *
 * 当命令队列状态发生变化时触发
 */
class CommandQueueStateEvent : public Event {
 public:
  enum class QueueState {
    BATCH_START,    // 开始批量命令
    BATCH_END,      // 结束批量命令
    QUEUE_PAUSED,   // 命令队列暂停
    QUEUE_RESUMED,  // 命令队列恢复
    QUEUE_CLEARED   // 命令队列清空
  };
  /**
   * @brief 命令队列状态事件构造函数
   * @param state 队列状态
   * @param batchName 批量操作名称（可选）
   * @param commandCount 队列中命令数量
   */
  explicit CommandQueueStateEvent(QueueState state,
                                  const std::string &batchName = "",
                                  size_t commandCount = 0)
      : m_State(state), m_BatchName(batchName), m_CommandCount(commandCount)
  {
  }

  QueueState GetState() const
  {
    return m_State;
  }
  const std::string &GetBatchName() const
  {
    return m_BatchName;
  }
  size_t GetCommandCount() const
  {
    return m_CommandCount;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_COMMAND)
  Event *Clone() const override
  {
    return new CommandQueueStateEvent(m_State, m_BatchName, m_CommandCount);
  }

 private:
  QueueState m_State;
  std::string m_BatchName;
  size_t m_CommandCount;
};

};  // namespace mite

#endif  // MITE_COMMAND_EVENTS
