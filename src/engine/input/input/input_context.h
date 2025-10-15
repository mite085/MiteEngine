#ifndef MITE_INPUT_CONTEXT
#define MITE_INPUT_CONTEXT

#include "input_event.h"

namespace mite {
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
  explicit InputContext(const std::string &name) : m_Name(name) {}
  ~InputContext() = default;

  // 基础属性
  const std::string &GetName() const { return m_Name; }
  void SetBlockInput(bool block) { m_BlockInput = block; }
  bool IsInputBlocked() const { return m_BlockInput; }

  // 输入处理: 按照优先级对事件进行排序，随后按顺序处理
  virtual void ProcessEvent(Event &e);

 protected:
  // 内部处理方法
  virtual void ProcessMouseMoveEvent(MouseMoveEvent &e) = 0;
  virtual void ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e) = 0;
  virtual void ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e) = 0;
  virtual void ProcessMouseScrollEvent(MouseScrollEvent &e) = 0;
  virtual void ProcessKeyPressdEvent(KeyPressedEvent &e) = 0;
  virtual void ProcessKeyReleasedEvent(KeyReleasedEvent &e) = 0;
  virtual void ProcessKeyTypedEvent(KeyTypedEvent &e) = 0;

  // 名称和阻塞状态
  std::string m_Name;
  bool m_BlockInput = false;
};

/**
 * @brief 输入上下文创建事件
 */
class InputContextCreateEvent : public Event {
 public:
  InputContextCreateEvent(std::shared_ptr<InputContext> context) : m_Context(context) {}
  std::shared_ptr<InputContext> GetContext() const { return m_Context; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SYSTEM)
  Event *Clone() const override { return new InputContextCreateEvent(m_Context); }

 private:
  std::shared_ptr<InputContext> m_Context;
};
};  // namespace mite

#endif
