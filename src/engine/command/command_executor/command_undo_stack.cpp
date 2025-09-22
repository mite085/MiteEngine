#include "command_undo_stack.h"

namespace mite {
CommandUndoStack::CommandUndoStack()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Undo Stack");
  m_Logger->debug("Command Undo Stack created");
}
void CommandUndoStack::Push(CommandPtr command)
{
  if (!command) {
    m_Logger->warn("Attempted to push null command to undo stack");
    return;
  }

  if (!command->CanUndo()) {
    m_Logger->warn("Command '{}' cannot be undone, not pushing to stack", command->GetName());
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  m_stack.push(std::move(command));
  m_Logger->debug("Command pushed to undo stack, size: {}", m_stack.size());
}

CommandPtr CommandUndoStack::Pop()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_stack.empty()) {
    m_Logger->debug("Undo stack is empty");
    return nullptr;
  }

  CommandPtr command = std::move(m_stack.top());
  m_stack.pop();

  m_Logger->debug("Command popped from undo stack, size: {}", m_stack.size());
  return command;
}

bool CommandUndoStack::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.empty();
}

size_t CommandUndoStack::GetSize() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.size();
}

void CommandUndoStack::Clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_Logger->debug("Clearing undo stack, size: {}", m_stack.size());

  // 使用空的栈替换当前栈来清空
  std::stack<CommandPtr> emptyStack;
  std::swap(m_stack, emptyStack);
}

Command *CommandUndoStack::Peek() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.empty() ? nullptr : m_stack.top().get();
}

}  // namespace mite
