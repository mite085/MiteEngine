#include "component.h"

namespace mite {
// ���෽���Ļ���ʵ��
// ע�⣺�󲿷ֹ�������ͷ�ļ���ʵ��Ϊ���麯��

void Component::SetDirty(bool dirty)
{
  m_Dirty = dirty;
}

bool Component::IsDirty() const
{
  return m_Dirty;
}

void Component::SetEnabled(bool enabled)
{
  m_Enabled = enabled;
}

bool Component::IsEnabled() const
{
  return m_Enabled;
}

bool Component::Serialize(std::ostream &output) const
{
  // ����������л�ֻд������״̬
  output.write(reinterpret_cast<const char *>(&m_Enabled), sizeof(m_Enabled));
  return !output.fail();
}

bool Component::Deserialize(std::istream &input)
{
  // ������������л�ֻ��ȡ����״̬
  input.read(reinterpret_cast<char *>(&m_Enabled), sizeof(m_Enabled));
  return !input.fail();
}
};  // namespace mite