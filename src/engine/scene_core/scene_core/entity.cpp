#include "entity.h"

namespace mite {
Entity::Entity() : m_UUID(UUID{}) {}
Entity::Entity(const UUID &uuid) : m_UUID(uuid) {}
Entity::Entity(const Entity &other) : m_UUID(other.m_UUID) {}
Entity Entity::CreateEntity()
{
  return Entity(UUIDGenerator::Generate());
}
bool Entity::IsValid() const
{
  return !m_UUID.is_nil();
}
void Entity::Destroy()
{
  m_UUID = UUID();  // …Ë÷√Œ™nil UUID
}
};  // namespace mite