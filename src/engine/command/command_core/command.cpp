#include "command.h"
#include "event_bus.h"
#include "command_event.h"

namespace mite {

// ==================== 命令完成处理 ====================
void Command::complete(CommandResult result)
{
  // 保存旧状态用于事件通知
  CommandExecutionState oldState = m_state;

  // 更新状态
  m_state = result.state;

  // 触发状态变更事件
  notifyStateChanged(oldState, m_state);

  // 执行回调函数
  if (m_callback) {
    m_callback(result);
  }

  // 发布命令完成事件
  EventBus::Publish<CommandCompletedEvent>(CommandCompletedEvent(this, result));
}
// ==================== 状态变更通知 ====================
void Command::notifyStateChanged(CommandExecutionState oldState, CommandExecutionState newState)
{
  // 只有状态真正变化时才发布事件
  if (oldState != newState) {
    EventBus::Publish<CommandStateChangedEvent>(
        CommandStateChangedEvent(this, oldState, newState));
  }
}
}  // namespace mite