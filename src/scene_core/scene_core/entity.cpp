#include "entity.h"
#include "scene.h"
#include "scene_core_components/component_headers.h"
#include "scene_registry.h"

namespace mite {
Entity::Entity(std::weak_ptr<Scene> scene, entt::entity handle) : m_Scene(scene), m_Handle(handle)
{
}

Entity::Entity(const Entity &other)
    : m_Scene(other.m_Scene),   // weak_ptr拷贝是安全的
      m_Handle(other.m_Handle)  // entt::entity可以直接拷贝
{
}

// 实体状态操作 ==============================================

bool Entity::IsValid() const
{
  if (m_Handle == entt::null)
    return false;
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->IsValid(*this);
  }
  return false;
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
  return m_Handle == other.m_Handle && m_Scene.lock() == other.m_Scene.lock();
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