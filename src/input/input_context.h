#ifndef MITE_INPUT_CONTEXT
#define MITE_INPUT_CONTEXT

#include "input_define.h"
#include "input_event.h"

namespace mite {
class InputAction {
 public:
  struct Binding {
    InputDevice device;
    int code;            // �����ť��
    float scale = 1.0f;  // ��������

    Binding(InputDevice device, int code, float scale = 1.0f);
  };

  std::string name;
  std::vector<Binding> bindings;
  float value = 0.0f;  // ��ǰ����ֵ
};

class InputContext {
 public:
  InputContext(const std::string &name);

  const std::string &GetName() const;
  bool IsInputBlocked() const;
  void BlockInput(bool block);
  

  // �����¼�������
  bool ProcessEvent(Event &event);

  void AddAction(const InputAction &action);
  void RemoveAction(const std::string &actionName);
  InputAction *GetAction(const std::string &actionName);

private:
  // �ڲ�������
  bool _ProcessKeyEvent(const KeyEvent &e);
  bool _ProcessMouseEvent(const MouseButtonEvent &e);
  void _UpdateActionValue(const std::string &actionName, float newValue);

 private:
  std::string m_Name;
  std::unordered_map<std::string, InputAction> m_Actions;
  bool m_BlockInput = false;

  Logger m_Logger;
};

};  // namespace mite

#endif
