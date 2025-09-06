#include "id_component.h"
#define UUID_SYSTEM_GENERATOR

namespace mite {
IDComponent::IDComponent()
    : ComponentTraits(), m_UUID(UUIDGenerator::Generate()), m_UUIDString(UUIDToString(m_UUID))
{
  // 确保生成的UUID有效
  if (m_UUID.is_nil()) {
    throw std::runtime_error("Failed to generate valid UUID");
  }
}

IDComponent::IDComponent(const std::string &id) : ComponentTraits()
{
  // 尝试解析字符串
  auto optionalUUID = UUID::from_string(id);
  if (!optionalUUID) {
    throw std::runtime_error("Invalid UUID string: {" + id + "}");
  }

  m_UUID = *optionalUUID;
  m_UUIDString = id;  // 使用原始字符串保持格式一致

  // 标准化字符串表示（小写、无花括号等）
  m_UUIDString = UUIDToString(m_UUID);
}

bool IDComponent::IsValid(const std::string &id)
{
  // 空字符串不算有效UUID
  if (id.empty())
    return false;

  // 尝试解析
  auto optionalUUID = UUID::from_string(id);
  return optionalUUID.has_value() && !optionalUUID->is_nil();
}
};  // namespace mite