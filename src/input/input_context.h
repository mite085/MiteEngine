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

class InputContextStack {
 public:
  InputContextStack();

  // �����������ģ�ջ����Ч��
  void Push(const std::shared_ptr<InputContext>& context);

  // ����ջ��������
  void Pop();

  // ��ȡ��ǰ��Ч��������
  std::shared_ptr<InputContext> GetCurrent();

  // ����Ƿ����ض���������
  bool IsInContext(const std::string &name);

  // ���������¼��������Ƿ����ѣ�
  bool ProcessEvent(Event &event);

  // ��ѯ�Ƿ��ջ
  bool IsEmpty();

  // ���
  void Clear();

 private:
  std::vector<std::shared_ptr<InputContext>> m_Stack;
  std::mutex m_Mutex;
  Logger m_Logger;
};

};  // namespace mite

#endif
