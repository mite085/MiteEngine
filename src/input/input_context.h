#ifndef MITE_INPUT_CONTEXT
#define MITE_INPUT_CONTEXT

#include "headers.h"
#include "input_define.h"
#include "input_event.h"

namespace mite {
class InputAction {
 public:
  struct Binding {
    InputDevice device;
    int code;            // 键码或按钮码
    float scale = 1.0f;  // 输入缩放

    Binding(InputDevice device, int code, float scale = 1.0f);
  };

  std::string name;
  std::vector<Binding> bindings;
  float value = 0.0f;  // 当前动作值
};

class InputContext {
 public:
  InputContext(const std::string &name);

  const std::string &GetName() const;
  bool IsInputBlocked() const;
  void BlockInput(bool block);
  

  // 核心事件处理方法
  bool ProcessEvent(Event &event);

  void AddAction(const InputAction &action);
  void RemoveAction(const std::string &actionName);
  InputAction *GetAction(const std::string &actionName);

private:
  // 内部处理方法
  bool _ProcessKeyEvent(const KeyEvent &e);
  bool _ProcessMouseEvent(const MouseButtonEvent &e);
  void _UpdateActionValue(const std::string &actionName, float newValue);

 private:
  std::string m_Name;
  std::unordered_map<std::string, InputAction> m_Actions;
  bool m_BlockInput = false;

  Logger m_Logger;
};

class InputContextStack {
 public:
  InputContextStack();

  // 推入新上下文（栈顶生效）
  void Push(const std::shared_ptr<InputContext>& context);

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

};  // namespace mite

#endif
