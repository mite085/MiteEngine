#include "entity.h"

namespace mite {
Entity::Entity() : m_UUID(uuids::uuid{}) {}
Entity::Entity(const uuids::uuid &uuid) : m_UUID(uuid) {}
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
  m_UUID = uuids::uuid();  // ÉèÖÃÎªnil UUID
}
};  // namespace mite