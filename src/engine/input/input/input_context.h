#ifndef MITE_INPUT_CONTEXT
#define MITE_INPUT_CONTEXT

#include "input_define.h"
#include "input_event.h"
#include "input_processor.h"

namespace mite {
class InputAction {
 public:
  struct Binding {
    InputDevice device;
    int code;            // 键码或按钮码
    float scale = 1.0f;  // 输入缩放

    Binding(InputDevice device, int code, float scale = 1.0f)
        : device(device), code(code), scale(scale)
    {
    }
  };

  std::string name;
  std::vector<Binding> bindings;
  float value = 0.0f;  // 当前动作值

  float holdTime = 0.0f;  // 长按计时
};
/**
 * @brief 输入上下文
 *
 * 功能：
 * 1. 基础输入管理
 * 2. 动作映射系统（Action Mapping）
 * 3. 事件处理（TODO：判断是否和Modular互相冲突？）
 */
class InputContext {
 public:
  explicit InputContext(const std::string &name);
  ~InputContext();

  // 基础属性
  const std::string &GetName() const;
  void SetBlockInput(bool block);
  bool IsInputBlocked() const;

  // 动作映射系统
  void AddAction(const InputAction &action);
  void RemoveAction(const std::string &name);
  InputAction *GetAction(const std::string &actionName);
  float GetActionValue(const std::string &name) const;

  // 每帧更新
  void Update();

  // 输入处理: 按照优先级对事件进行排序，随后按顺序处理
  virtual bool ProcessEvent(Event &e) = 0;

  // 调试工具
  void DebugPrintActions() const;

 protected:
  // 内部处理方法
  bool _ProcessKeyPressedEvent(const KeyPressedEvent &e);
  bool _ProcessMouseButtonPressedEvent(const MouseButtonPressedEvent &e);
  bool _ProcessMouseMoveEvent(const MouseMoveEvent &e);
  bool _ProcessMouseScrollEvent(const MouseScrollEvent &e);
  void _UpdateActionValue(const std::string &actionName, float newValue);

  std::string m_Name;
  bool m_BlockInput = false;

  // 动作系统
  std::unordered_map<std::string, InputAction> m_Actions;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};
};  // namespace mite

#endif
