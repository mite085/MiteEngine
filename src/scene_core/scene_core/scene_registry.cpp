#include "scene_registry.h"
#include "scene_core_components/component_headers.h"

namespace mite {
SceneRegistry::SceneRegistry(std::weak_ptr<Scene> scene) : m_Scene(scene) {
  // 注册基础Component类型的回调
  m_Registry.on_construct<Component>().connect<&SceneRegistry::FireConstructEvent>(this);
  m_Registry.on_update<Component>().connect<&SceneRegistry::FireUpdateEvent>(this);
  m_Registry.on_destroy<Component>().connect<&SceneRegistry::FireDestroyEvent>(this);
}

SceneRegistry::~SceneRegistry()
{  // 断开所有回调
  m_Registry.on_construct<Component>().disconnect(this);
  m_Registry.on_update<Component>().disconnect(this);
  m_Registry.on_destroy<Component>().disconnect(this);
}

Entity SceneRegistry::CreateEntity(const std::string &name)
{
  Entity entity = Entity{m_Scene, m_Registry.create()};
  
  // 添加基本组件，自动生成唯一ID
  auto &id = entity.AddComponent<IDComponent>();

  // 添加Tag系统，用于实体搜索和筛选
  auto &tag = entity.AddComponent<TagComponent>();
  tag.SetTag(name.empty() ? "Entity_" + id.String() : name);

  return entity;
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

template<typename... Component>
std::vector<Entity> SceneRegistry::GetEntitiesWith()
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
  assert(IsValid(entity));
  if (HasComponent<T>(entity)) {
    RemoveComponent<T>(entity);
  }
  return m_Registry.emplace<T>(entity.GetHandle(), std::forward<Args>(args)...);
}

template<typename T> T &SceneRegistry::GetOrAddComponent(Entity entity)
{
  assert(IsValid(entity));
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
  assert(HasComponent<T>(entity));
  return m_Registry.get<T>(entity.GetHandle());
}

template<typename T> const T &SceneRegistry::GetComponent(Entity entity) const
{
  assert(HasComponent<T>(entity));
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

template<typename... Component> bool SceneRegistry::AnyOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.any_of<Component...>(entity.GetHandle());
}

template<typename... Component> bool SceneRegistry::AllOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.all_of<Component...>(entity.GetHandle());
}

template<typename T> void SceneRegistry::OnComponentConstruct(ComponentConstructCallback callback)
{
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  const std::type_index type = typeid(T);
  m_ConstructCallbacks[type] = callback;

  // 连接到EnTT的回调系统
  m_Registry.on_construct<T>().template connect<&SceneRegistry::InvokeConstruct<T>>(this);
}

template<typename T> void SceneRegistry::OnComponentUpdate(ComponentUpdateCallback callback)
{
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  const std::type_index type = typeid(T);
  m_UpdateCallbacks[type] = callback;

  // 连接到EnTT的回调系统
  m_Registry.on_update<T>().template connect<&SceneRegistry::InvokeUpdate<T>>(this);
}

template<typename T> void SceneRegistry::OnComponentDestroy(ComponentDestroyCallback callback)
{
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  const std::type_index type = typeid(T);
  m_DestroyCallbacks[type] = callback;

  // 连接到EnTT的回调系统
  m_Registry.on_destroy<T>().template connect<&SceneRegistry::InvokeDestroy<T>>(this);
}

template<typename T> void SceneRegistry::InvokeConstruct(Entity entity, T &component)
{
  const std::type_index type = typeid(T);
  if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
    it->second(entity, component);
  }
}

template<typename T> void SceneRegistry::InvokeUpdate(Entity entity, T &component)
{
  const std::type_index type = typeid(T);
  if (auto it = m_UpdateCallbacks.find(type); it != m_UpdateCallbacks.end()) {
    it->second(entity, component);
  }
}

template<typename T> void SceneRegistry::InvokeDestroy(Entity entity, T &component)
{
  const std::type_index type = typeid(T);
  if (auto it = m_DestroyCallbacks.find(type); it != m_DestroyCallbacks.end()) {
    it->second(entity, component);
  }
}

};  // namespace mite