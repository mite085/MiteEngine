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
    int code;            // �����ť��
    float scale = 1.0f;  // ��������

    Binding(InputDevice device, int code, float scale = 1.0f)
        : device(device), code(code), scale(scale)
    {
    }
  };

  std::string name;
  std::vector<Binding> bindings;
  float value = 0.0f;  // ��ǰ����ֵ

  float hold_time = 0.0f;  // ������ʱ
};

// class InputContext {
//  public:
//   InputContext(const std::string &name);
//
//   const std::string &GetName() const;
//   bool IsInputBlocked() const;
//   void BlockInput(bool block);
//
//
//   // �����¼�������
//   bool ProcessEvent(Event &event);
//
//   void AddAction(const InputAction &action);
//   void RemoveAction(const std::string &actionName);
//   InputAction *GetAction(const std::string &actionName);
//
// private:
//   // �ڲ�������
//   bool _ProcessKeyEvent(const KeyEvent &e);
//   bool _ProcessMouseEvent(const MouseButtonEvent &e);
//   void _UpdateActionValue(const std::string &actionName, float newValue);
//
//  private:
//   std::string m_Name;
//   std::unordered_map<std::string, InputAction> m_Actions;
//   bool m_BlockInput = false;
//
//   Logger m_Logger;
// };

class InputContext {
 public:
  explicit InputContext(const std::string &name);
  ~InputContext();

  // ��������
  const std::string &GetName() const;
  void SetBlockInput(bool block);
  bool IsInputBlocked() const;

  // ����ӳ��ϵͳ
  void AddAction(const InputAction &action);
  void RemoveAction(const std::string &name);
  InputAction *GetAction(const std::string &actionName);
  float GetActionValue(const std::string &name) const;

  // ���Ĵ�������
  virtual bool ProcessEvent(Event &event);
  void Update();

  // ���Թ���
  void DebugPrintActions() const;

 protected:
  // �ڲ�������
  bool _ProcessKeyEvent(const KeyEvent &e);
  bool _ProcessMouseButtonEvent(const MouseButtonEvent &e);
  bool _ProcessMouseMoveEvent(const MouseMoveEvent &e);
  // bool _ProcessMouseScrollEvent(const MouseScrollEvent &e);
  void _UpdateActionValue(const std::string &actionName, float newValue);

  //void _SortProcessors();

  std::string m_Name;
  bool m_BlockInput = false;

  // ����ϵͳ
  std::unordered_map<std::string, InputAction> m_Actions;

  Logger m_Logger;
};
};  // namespace mite

#endif
