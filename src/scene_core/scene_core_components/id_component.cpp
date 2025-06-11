#include "id_component.h"

namespace mite {

// UUID�ַ���ת���ı��ظ�������
std::string UUIDToString(const uuids::uuid &id)
{
  return uuids::to_string(id);
}

IDComponent::IDComponent()
    : m_UUID(uuids::uuid_system_generator{}())  // ʹ��ϵͳ�����������
      ,
      m_UUIDString(UUIDToString(m_UUID))
{
  // ȷ�����ɵ�UUID��Ч
  if (m_UUID.is_nil()) {
    throw std::runtime_error("Failed to generate valid UUID");
  }
}

IDComponent::IDComponent(const std::string &id)
{
  // ���Խ����ַ���
  auto optionalUUID = uuids::uuid::from_string(id);
  if (!optionalUUID) {
    throw std::runtime_error("Invalid UUID string: {" + id + "}");
  }

  m_UUID = *optionalUUID;
  m_UUIDString = id;  // ʹ��ԭʼ�ַ������ָ�ʽһ��

  // ��׼���ַ�����ʾ��Сд���޻����ŵȣ�
  m_UUIDString = UUIDToString(m_UUID);
}

IDComponent IDComponent::Generate()
{
  return IDComponent();  // ί�и�Ĭ�Ϲ��캯��
}

bool IDComponent::IsValid(const std::string &id)
{
  // ���ַ���������ЧUUID
  if (id.empty())
    return false;

  // ���Խ���
  auto optionalUUID = uuids::uuid::from_string(id);
  return optionalUUID.has_value() && !optionalUUID->is_nil();
}
};
