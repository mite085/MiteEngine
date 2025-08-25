#include "component_id.h"
#include "scene_core_components/component_headers.h"

namespace mite {

ComponentID::ComponentID(uuids::uuid id) : m_ID(std::move(id)) {}

template<typename T> ComponentID ComponentID::Get()
{
  // 使用类型信息生成确定性UUID
  const std::type_index typeIdx(typeid(T));
  const size_t hash = typeIdx.hash_code();

  // 静态局部变量保证每个类型只有一个ID实例
  static const ComponentID id(UUIDGenerator::Generate(hash));
  return id;
}

ComponentID ComponentID::FromString(const std::string &uuidStr)
{
  auto result = uuids::uuid::from_string(uuidStr);
  if (result) {
    return ComponentID(*result);
  }
  return ComponentID(uuids::uuid());
}

bool ComponentID::operator==(const ComponentID &other) const
{
  return m_ID == other.m_ID;
}

bool ComponentID::operator!=(const ComponentID &other) const
{
  return !(*this == other);
}

bool ComponentID::operator<(const ComponentID &other) const
{
  return m_ID < other.m_ID;
}

std::string ComponentID::ToString() const
{
  return uuids::to_string(m_ID);
}

size_t ComponentID::Hash() const
{
  return std::hash<uuids::uuid>{}(m_ID);
}

bool ComponentID::IsValid() const
{
  return !m_ID.is_nil();
}

// 显式实例化常用组件类型的ID
template ComponentID ComponentID::Get<CameraComponent>();
template ComponentID ComponentID::Get<DestroyComponent>();
template ComponentID ComponentID::Get<HierarchyComponent>();
template ComponentID ComponentID::Get<IDComponent>();
template ComponentID ComponentID::Get<MaterialComponent>();
template ComponentID ComponentID::Get<MeshComponent>();
template ComponentID ComponentID::Get<TagComponent>();
template ComponentID ComponentID::Get<TransformComponent>();

// VisibilityComponent隶属于SceneGraph模块，应当在SceneGraph中实现
class VisibilityComponent;                                       // 前向声明
template<> ComponentID ComponentID::Get<VisibilityComponent>();  // 声明显式特化

};
