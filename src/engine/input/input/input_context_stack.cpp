#include "input_context_stack.h"

namespace mite {
InputContextStack::InputContextStack()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Context Stack");
  m_Logger->debug("Created input context stack");
}

void InputContextStack::Push(const std::shared_ptr<InputContext> &context)
{
  if (!context) {
    m_Logger->error("Attempted to push null InputContext!");
    return;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // 防止重复推入相同实例
  if (!m_Stack.empty() && m_Stack.back() == context) {
    m_Logger->warn("Duplicate context pushed: {}", context->GetName());
    return;
  }

  m_Stack.push_back(context);
  m_Logger->trace("Context pushed: {}", context->GetName());
}

void InputContextStack::Pop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Stack.empty()) {
    m_Logger->error("Attempted to pop empty InputContextStack!");
    return;
  }

  m_Logger->trace("Context popped: {}", m_Stack.back()->GetName());
  m_Stack.pop_back();
}

std::shared_ptr<InputContext> InputContextStack::GetCurrent()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Stack.empty()) {
    return nullptr;
  }
  return m_Stack.back();
}

bool InputContextStack::IsInContext(const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 从栈顶向下搜索
  auto it = std::find_if(
      m_Stack.rbegin(), m_Stack.rend(), [&name](const std::shared_ptr<InputContext> &ctx) {
        return ctx->GetName() == name;
      });

  return it != m_Stack.rend();
}

bool InputContextStack::ProcessEvent(Event &event)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 从栈顶向下处理
  for (auto it = m_Stack.rbegin(); it != m_Stack.rend(); ++it) {
    const auto &context = *it;

    // 1. 检查是否阻塞输入
    if (context->IsInputBlocked()) {
      m_Logger->trace("Event blocked by context: {}", context->GetName());
      return true;
    }

    // 2. 尝试处理事件
    if (context->ProcessEvent(event)) {
      m_Logger->trace("Event consumed by context: {}", context->GetName());
      return true;
    }
  }

  return false;
}

bool InputContextStack::IsEmpty()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stack.empty();
}

// 辅助方法：清空栈（用于场景切换等）
void InputContextStack::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Logger->info("Clearing all {} input contexts", m_Stack.size());
  m_Stack.clear();
}
};
