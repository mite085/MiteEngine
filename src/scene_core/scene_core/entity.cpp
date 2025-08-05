#include "entity.h"
#include "scene.h"
#include "scene_core_components/component_headers.h"
#include "scene_registry.h"

namespace mite {
Entity::Entity(entt::entity handle) :m_Handle(handle)
{
}

Entity::Entity(const Entity &other)
    : m_Handle(other.m_Handle)  // entt::entity可以直接拷贝
{
}

// 实体状态操作 ==============================================

bool Entity::IsValid() const
{
  if (m_Handle == entt::null)
    return false;
  else
    return true;
}

void Entity::Destroy()
{
  if (!IsValid())
    return;

  m_Handle = entt::null;
}

entt::entity Entity::GetHandle() const
{
  return m_Handle;
}

// 操作符重载 ===============================================

bool Entity::operator==(const Entity &other) const
{
  return m_Handle == other.m_Handle;
}

bool Entity::operator!=(const Entity &other) const
{
  return !(*this == other);
}

Entity::operator bool() const
{
  return IsValid();
}

// std::shared_ptr<Scene> Entity::GetScene() const
//{
//   return m_Scene.lock();
// }
};  // namespace mite