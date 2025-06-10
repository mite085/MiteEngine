#ifndef MITE_INPUT_PROCESSOR
#define MITE_INPUT_PROCESSOR

#include "headers.h"

namespace mite {
// ���������ȼ�����
namespace InputPriority {
const int SYSTEM = 1000;   // ϵͳ����ݼ�
const int UI_MODAL = 800;  // ģ̬�Ի���
const int UI_FORM = 500;   // ����UI�ؼ�
const int GAMEPLAY = 300;  // ��Ϸ�߼�
const int CAMERA = 200;    // ���������
}  // namespace InputPriority

class InputProcessor {
 public:
  virtual ~InputProcessor() = default;

  // ���Ĵ���ӿ�
  virtual bool HandleEvent(Event &e) = 0;

  // ���ýӿ�
  virtual int GetPriority() const = 0;
  virtual const std::string &GetID() const = 0;
  virtual bool IsEnabled() const
  {
    return m_Enabled;
  }
  virtual void SetEnabled(bool enabled)
  {
    m_Enabled = enabled;
  }

 protected:
  bool m_Enabled = true;
};
};

#endif
