#include "command_redo_stack.h"

namespace mite {
CommandRedoStack::CommandRedoStack()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Redo Stack");
  m_Logger->debug("Command Redo Stack created");
}
void CommandRedoStack::Push(CommandPtr command)
{
  if (!command) {
    m_Logger->warn("Attempted to push null command to redo stack");
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  m_stack.push(std::move(command));
  m_Logger->debug("Command pushed to redo stack, size: {}", m_stack.size());
}

CommandPtr CommandRedoStack::Pop()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_stack.empty()) {
    m_Logger->debug("Redo stack is empty");
    return nullptr;
  }

  CommandPtr command = std::move(m_stack.top());
  m_stack.pop();

  m_Logger->debug("Command popped from redo stack, size: {}", m_stack.size());
  return command;
}

bool CommandRedoStack::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.empty();
}

size_t CommandRedoStack::GetSize() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.size();
}

void CommandRedoStack::Clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_Logger->debug("Clearing redo stack, size: {}", m_stack.size());

  // 使用空的栈替换当前栈来清空
  std::stack<CommandPtr> emptyStack;
  std::swap(m_stack, emptyStack);
}

Command *CommandRedoStack::Peek() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_stack.empty() ? nullptr : m_stack.top().get();
}

}  // namespace mite
