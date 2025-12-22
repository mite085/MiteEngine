#ifndef MITE_INPUT_MANAGER
#define MITE_INPUT_MANAGER

#include "input_context_stack.h"

namespace mite {
// InputManager是输入模块的核心实现类，
// 负责统一管理所有输入设备的状态、
// 处理输入事件的分发逻辑，并维护输入上下文栈。
class InputManager {
 public:
  static InputManager &Get();

  InputManager() = default;
  ~InputManager() = default;

  void Init();
  void Shutdown();

  // 上下文管理
  void PushContext(std::shared_ptr<InputContext> context);
  void PopContext();
  std::shared_ptr<InputContext> GetCurrentContext();

 private:
  // 事件通用入口
  void ProcessEvent(Event &e);
  void OnInputContextCreate(InputContextCreateEvent &e);

  // 输入上下文栈
  std::shared_ptr<InputContextStack> m_ContextStack;

  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};
};  // namespace mite

#endif
