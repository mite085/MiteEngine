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

  // 处理输入事件（返回是否被消费）
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
