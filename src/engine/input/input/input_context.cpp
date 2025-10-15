#include "input_context.h"

namespace mite {
void InputContext::ProcessEvent(Event &e)
{
  // 若为阻塞状态，应当停止传播（这一步在InputStack执行，此处无需考虑）
  if (m_BlockInput) {
    e.SetResult(EventResult::Blocked);
    return;
  }
  // 分发事件
  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<MouseMoveEvent>(BIND_DISPATCH_FN(ProcessMouseMoveEvent));
  dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DISPATCH_FN(ProcessMouseButtonPressedEvent));
  dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DISPATCH_FN(ProcessMouseButtonReleasedEvent));
  dispatcher.Dispatch<MouseScrollEvent>(BIND_DISPATCH_FN(ProcessMouseScrollEvent));
  dispatcher.Dispatch<KeyPressedEvent>(BIND_DISPATCH_FN(ProcessKeyPressdEvent));
  dispatcher.Dispatch<KeyReleasedEvent>(BIND_DISPATCH_FN(ProcessKeyReleasedEvent));
  dispatcher.Dispatch<KeyTypedEvent>(BIND_DISPATCH_FN(ProcessKeyTypedEvent));
}
};  // namespace mite