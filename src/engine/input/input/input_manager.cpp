#include "input_manager.h"

namespace mite {
void InputManager::Init()
{
  // 创建输入上下文栈
  m_ContextStack = std::make_shared<InputContextStack>();

  // 按大类订阅所有输入事件，由InputManager统一分发
  // Immediate同步模式：
  // 输入事件需要立即处理以确保响应的实时性，避免输入延迟影响体验
  m_EventSubscriptions.SubscribeByCategoryImmediate(
      EventCategory::EVENT_CATEGORY_INPUT, BIND_DISPATCH_FN(ProcessEvent), EventPriority::Highest);

  // 订阅输入上下文创建事件，直接进行入栈操作
  m_EventSubscriptions.SubscribeImmediate<InputContextCreateEvent>(
      BIND_DISPATCH_FN(OnInputContextCreate));
}

void InputManager::Shutdown()
{
  // 清理所有上下文
  m_ContextStack->Clear();
}

void InputManager::PushContext(std::shared_ptr<InputContext> context)
{
  if (!context) {
    LOG_ERROR("Attempted to push null input context");
    return;
  }

  // 非空栈情况下，
  // 日志记录Input输入的入栈顺序
  if (!m_ContextStack->IsEmpty()) {
    auto &current = m_ContextStack->GetCurrent();
    LOG_DEBUG("Pushing input context '{}' over '{}'", context->GetName(), current->GetName());
  }
  else {
    LOG_DEBUG("Pushing first input context '{}'", context->GetName());
  }

  // 执行入栈操作
  m_ContextStack->Push(context);
}

void InputManager::PopContext()
{
  if (m_ContextStack->IsEmpty()) {
    LOG_ERROR("Attempted to pop empty input context stack");
    return;
  }

  // 执行出栈操作
  auto popped = m_ContextStack->GetCurrent();
  m_ContextStack->Pop();

  // 出栈后非空栈的情况下，
  // 日志记录下一个Input事件
  if (!m_ContextStack->IsEmpty()) {
    auto &newCurrent = m_ContextStack->GetCurrent();
    LOG_DEBUG("Popped input context '{}', new current is '{}'",
              popped->GetName(),
              newCurrent->GetName());
  }
  else {
    LOG_DEBUG("Popped last input context '{}'", popped->GetName());
  }
}

std::shared_ptr<InputContext> InputManager::GetCurrentContext()
{
  // 空栈情况下访问当前上下文
  if (m_ContextStack->IsEmpty()) {
    LOG_WARN("No input context available");
    return nullptr;
  }

  return m_ContextStack->GetCurrent();
}

void InputManager::ProcessEvent(Event &e)
{
  m_ContextStack->ProcessEvent(e);
}
void InputManager::OnInputContextCreate(InputContextCreateEvent &e)
{
  PushContext(e.GetContext());

  e.SetResult(EventResult::HandledAndStop);
  return;
}
};  // namespace mite