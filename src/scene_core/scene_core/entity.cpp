#include "entity.h"

namespace mite {
Entity::Entity() : m_UUID(UUIDGenerator::Generate()) {}
Entity::Entity(const uuids::uuid &uuid) : m_UUID(uuid) {}
Entity::Entity(const Entity &other) : m_UUID(other.m_UUID) {}
bool Entity::IsValid() const
{
  return !m_UUID.is_nil();
}
void Entity::Destroy()
{
  m_UUID = uuids::uuid();  // …Ë÷√Œ™nil UUID
}
};  // namespace mite