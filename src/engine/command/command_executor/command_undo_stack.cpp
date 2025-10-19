#include "command_undo_stack.h"

namespace mite {
CommandUndoStack::CommandUndoStack(size_t maxSize) : m_MaxSize(maxSize)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Command Undo Stack");
  m_Logger->debug("Command Undo Stack created");
}
void CommandUndoStack::Push(CommandRegistry &registry, CommandHandle commandHandle)
{
  if (!commandHandle.IsValid()) {
    m_Logger->warn("Attempted to push null command handle to undo stack");
    return;
  }
  // 通过注册表Peek命令是否可以撤销
  const Command *command = registry.PeekCommand(commandHandle);
  if (!command) {
    m_Logger->warn("Attempted to push invalid command handle to undo stack");
    return;
  }
  if (!command->CanUndo()) {
    m_Logger->warn("Command '{}' cannot be undone, not pushing to stack", command->GetName());
    return;
  }
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 检查最大大小限制（若为0则无限制，不执行移除操作）
  if (m_MaxSize > 0 && m_Stack.size() >= m_MaxSize) {
    // 移除栈底元素（需要将栈转换为队列来操作）
    std::stack<CommandHandle> tempStack;

    // 将除了第一个元素外的所有元素转移到临时栈
    while (m_Stack.size() > 1) {
      tempStack.push(std::move(m_Stack.top()));
      m_Stack.pop();
    }

    // 移除栈底元素（现在在栈顶）
    CommandHandle removedHandle = std::move(m_Stack.top());
    m_Stack.pop();

    // 恢复其他元素
    while (!tempStack.empty()) {
      m_Stack.push(std::move(tempStack.top()));
      tempStack.pop();
    }

    m_Logger->debug("Undo stack reached max size {}, removed oldest command", m_MaxSize);
  }

  m_Stack.push(std::move(commandHandle));
  m_Logger->debug("Command pushed to undo stack, size: {}", m_Stack.size());
}

CommandHandle CommandUndoStack::Pop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_Stack.empty()) {
    m_Logger->debug("Undo stack is empty");
    return CommandHandle();
  }
  CommandHandle commandHandle = std::move(m_Stack.top());
  m_Stack.pop();
  m_Logger->debug("Command popped from undo stack, size: {}", m_Stack.size());
  return commandHandle;
}
bool CommandUndoStack::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stack.empty();
}
size_t CommandUndoStack::GetSize() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stack.size();
}
size_t CommandUndoStack::GetMaxSize() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_MaxSize;
}
void CommandUndoStack::SetMaxSize(size_t maxSize)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (maxSize == m_MaxSize) {
    return;
  }

  m_MaxSize = maxSize;

  // 如果新的大小限制小于当前栈大小，需要移除多余的元素（若为0则无限制，不执行移除操作）
  if (m_MaxSize > 0 && m_Stack.size() > m_MaxSize) {
    size_t elementsToRemove = m_Stack.size() - m_MaxSize;
    std::stack<CommandHandle> tempStack;

    // 保留最新的 m_maxSize 个元素
    while (m_Stack.size() > m_MaxSize) {
      tempStack.push(std::move(m_Stack.top()));
      m_Stack.pop();
    }

    // 清空临时栈（移除多余的元素）
    while (!tempStack.empty()) {
      tempStack.pop();
    }

    m_Logger->debug("Undo stack resized from {} to {}, removed {} oldest commands",
                    m_Stack.size() + elementsToRemove,
                    m_MaxSize,
                    elementsToRemove);
  }
}
void CommandUndoStack::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Logger->debug("Clearing undo stack, size: {}", m_Stack.size());
  // 使用空的栈替换当前栈来清空
  std::stack<CommandHandle> emptyStack;
  std::swap(m_Stack, emptyStack);
}
CommandHandle CommandUndoStack::Peek() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stack.empty() ? CommandHandle() : m_Stack.top();
}
}  // namespace mite
