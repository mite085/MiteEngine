#include "scene_registry.h"
#include "scene_core_components/id_component.h"

namespace mite {
SceneRegistry::SceneRegistry(std::weak_ptr<Scene> scene) : m_Scene(scene) {}

Entity SceneRegistry::CreateEntity()
{
  auto entity = m_Registry.create();
  // 确保每个实体都有ID组件
  m_Registry.emplace<IDComponent>(entity);
  return Entity(m_Scene, entity);
}

void SceneRegistry::DestroyEntity(Entity entity)
{
  if (IsValid(entity)) {
    m_Registry.destroy(entity.GetHandle());
  }
}

bool SceneRegistry::IsValid(Entity entity) const
{
  return entity.IsValid() && m_Registry.valid(entity.GetHandle());
}

void SceneRegistry::Clear()
{
  m_Registry.clear();
}

std::vector<Entity> SceneRegistry::GetAllEntities()
{
  std::vector<Entity> entities;

  // 预留空间提高效率
  entities.reserve(m_Registry.storage<entt::entity>().size());

  // 遍历视图中的所有实体
  for (auto entity : m_Registry.storage<entt::entity>()) {
    if (m_Registry.valid(entity)) {
      entities.emplace_back(m_Scene, entity);
    }
  }

  return entities;
}

template<typename... Component> std::vector<Entity> SceneRegistry::GetEntitiesWith()
{
  std::vector<Entity> entities;
  auto view = m_Registry.view<Component...>();

  // 预留空间提高效率
  entities.reserve(view.size_hint());

  // 遍历视图中的所有实体
  for (auto entity : view) {
    if (m_Registry.valid(entity)) {
      entities.emplace_back(m_Scene, entity);
    }
  }

  return entities;
}

// 组件操作实现 ==============================================

template<typename T, typename... Args>
T &SceneRegistry::AddComponent(Entity entity, Args &&...args)
{
  assert(IsValid(entity), "Cannot add component to invalid entity!");
  if (HasComponent<T>(entity)) {
    RemoveComponent<T>(entity);
  }
  return m_Registry.emplace<T>(entity.GetHandle(), std::forward<Args>(args)...);
}

template<typename T> T &SceneRegistry::GetOrAddComponent(Entity entity)
{
  assert(IsValid(entity), "Cannot get component from invalid entity!");
  return m_Registry.get_or_emplace<T>(entity.GetHandle());
}

template<typename T> void SceneRegistry::RemoveComponent(Entity entity)
{
  if (IsValid(entity) && m_Registry.all_of<T>(entity.GetHandle())) {
    m_Registry.remove<T>(entity.GetHandle());
  }
}

template<typename T> bool SceneRegistry::HasComponent(Entity entity) const
{
  return IsValid(entity) && m_Registry.all_of<T>(entity.GetHandle());
}

template<typename T> T &SceneRegistry::GetComponent(Entity entity)
{
  assert(HasComponent<T>(entity), "Entity does not have component!");
  return m_Registry.get<T>(entity.GetHandle());
}

template<typename T> const T &SceneRegistry::GetComponent(Entity entity) const
{
  assert(HasComponent<T>(entity), "Entity does not have component!");
  return m_Registry.get<T>(entity.GetHandle());
}

template<typename T> T *SceneRegistry::TryGetComponent(Entity entity)
{
  if (!IsValid(entity))
    return nullptr;
  return m_Registry.try_get<T>(entity.GetHandle());
}

template<typename T> const T *SceneRegistry::TryGetComponent(Entity entity) const
{
  if (!IsValid(entity))
    return nullptr;
  return m_Registry.try_get<T>(entity.GetHandle());
}

// 视图和查询实现 ===========================================

template<typename... Component, typename... Exclude>
auto SceneRegistry::View(entt::exclude_t<Exclude...> exclude) const
{
  return m_Registry.view<Component...>(exclude);
}

template<typename... Component> bool SceneRegistry::AnyOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.any_of<Component...>(entity.GetHandle());
}

template<typename... Component> bool SceneRegistry::AllOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.all_of<Component...>(entity.GetHandle());
}
};  // namespace mite