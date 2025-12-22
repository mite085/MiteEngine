#include "entity.h"

namespace mite {
Entity::Entity() : m_Name(""), m_UUID(UUID{}) {}
Entity::Entity(const std::string &name, const UUID &uuid)
    : m_Name(name), m_UUID(uuid) {}
Entity::Entity(const Entity &other)
    : m_Name(other.GetName()), m_UUID(other.m_UUID) {}
Entity Entity::CreateEntity(const std::string &name) {
  return Entity(name, UUIDGenerator::Generate());
}
bool Entity::IsValid() const { return !m_UUID.is_nil(); }
void Entity::Destroy() {
  m_UUID = UUID();  // 设置为nil UUID
}
};  // namespace mite