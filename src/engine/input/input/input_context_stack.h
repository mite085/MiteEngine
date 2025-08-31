#ifndef MITE_INPUT_CONTEXT_STACK
#define MITE_INPUT_CONTEXT_STACK

#include "input_context.h"

namespace mite {

class InputContextStack {
 public:
  InputContextStack();

  // 推入新上下文（栈顶生效）
  void Push(const std::shared_ptr<InputContext> &context);

  // 弹出栈顶上下文
  void Pop();

  // 获取当前生效的上下文
  std::shared_ptr<InputContext> GetCurrent();

  // 检查是否在特定上下文中
  bool IsInContext(const std::string &name);

  // 处理输入事件
  // 注意：
  // 该功能暂时被ModularInputContext的SubscribeByCategory大类订阅功能替代，
  // 未正常启用（没有地方调用该函数）
  // 
  // InputContextStack负责了所有的InputContext，事件类型与数量可能极多，
  // 使用单一的ProcessEvent事件入口是否合适，每次事件都锁mutex是否会影响性能
  // 
  bool ProcessEvent(Event &event);

  // 查询是否空栈
  bool IsEmpty();

  // 清空
  void Clear();

 private:
  std::vector<std::shared_ptr<InputContext>> m_Stack;
  std::mutex m_Mutex;
  Logger m_Logger;
};
};

#endif
