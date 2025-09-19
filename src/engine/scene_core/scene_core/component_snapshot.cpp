#include "component_snapshot.h"

namespace mite {
ComponentSnapshot::ComponentSnapshot(Entity entityId, std::type_index componentType)
    : m_entityId(entityId),
      m_componentType(componentType),
      m_memoryUsage(0)
{
}

void ComponentSnapshot::Apply()
{
  DeserializeState();
}

void ComponentSnapshot::Revert()
{
  DeserializeState();
}

size_t ComponentSnapshot::GetMemoryUsage() const
{
  return m_memoryUsage;
}

const char *ComponentSnapshot::GetDescription() const
{
  return m_componentType.name();
}

}  // namespace mite::scene
