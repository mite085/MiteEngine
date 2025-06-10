#ifndef MITE_INPUT_CONTEXT_STACK
#define MITE_INPUT_CONTEXT_STACK

#include "input_context.h"

namespace mite {

class InputContextStack {
 public:
  InputContextStack();

  // �����������ģ�ջ����Ч��
  void Push(const std::shared_ptr<InputContext> &context);

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
};

#endif
