#include "scene_registry.h"
#include "scene_core_components/component_headers.h"

namespace mite {
SceneRegistry::SceneRegistry(std::weak_ptr<Scene> scene) : m_Scene(scene) {}

SceneRegistry::~SceneRegistry()
{  
  // 断开所有回调
  m_Registry.on_construct<Component>().disconnect(this);
  m_Registry.on_update<Component>().disconnect(this);
  m_Registry.on_destroy<Component>().disconnect(this);
}

// 1. 实体管理 ===================================================

Entity SceneRegistry::CreateEntity(const std::string name)
{
  // 使用entt::registry::create()创建实体
  Entity entity = Entity{m_Scene, m_Registry.create()};

  // 添加基本组件，自动生成唯一ID
  auto &id = AddComponent<IDComponent>(entity);

  // 添加Tag系统，用于实体搜索和筛选
  auto &tag = AddComponent<TagComponent>(entity);
  tag.SetTag(name.empty() ? "Entity_" + id.String() : name);

  // 主动触发回调函数
  ExecuteCallbacks(m_EntityCallbacks.createdCallbacks, entity);

  return entity;
}

void SceneRegistry::DestroyEntity(Entity entity)
{
  if (IsValid(entity)) {
    // 1. 主动触发PreDestroy回调（实体仍完整）
    ExecuteCallbacks(m_EntityCallbacks.preDestroyCallbacks, entity);

    // 2. 使用entt::registry::destroy销毁实体
    //
    // 注意：
    // 若该entity存在任何已经被
    // RegisterCallbackComponentDestroy
    // 注册的Component，会在这里触发回调，
    // 运行各个被注册的callback函数
    m_Registry.destroy(entity.GetHandle());

    // 3. 主动触发PostDestroy回调（实体已无效）
    ExecuteCallbacks(m_EntityCallbacks.postDestroyCallbacks, entity);
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

// 2. 组件操作 - 基础 ==============================================

template<typename T, typename... Args>
T &SceneRegistry::AddComponent(Entity entity, Args &&...args)
{
  // 使用Assert断言，确保entity有效
  assert(IsValid(entity));

  // 若该实体已经挂载了和当前添加组件
  // 同类型的组件，则移除原组件，
  // 以确保新组件正常添加
  if (HasComponent<T>(entity)) {
    RemoveComponent<T>(entity);
  }

  // 注意：若T已经被RegisterCallbackComponentConstruct注册，
  // 这里会触发entt内部的回调，运行被注册的callback函数
  return m_Registry.emplace<T>(entity.GetHandle(), std::forward<Args>(args)...);
}

template<typename T> T &SceneRegistry::GetOrAddComponent(Entity entity)
{
  assert(IsValid(entity));

  // 破坏性低于AddComponent的用法，
  // 原组件存在时不移除，直接返回原组件
  return m_Registry.get_or_emplace<T>(entity.GetHandle());
}

template<typename T> void SceneRegistry::RemoveComponent(Entity entity)
{
  // 销毁组件时，无需使用assert断言确保entity有效。
  if (IsValid(entity) && m_Registry.all_of<T>(entity.GetHandle())) {

    // 注意：若T已经被RegisterCallbackComponentDestroy注册，
    // 这里会触发回调，运行被注册的callback函数
    m_Registry.remove<T>(entity.GetHandle());
  }
}

template<typename T> bool SceneRegistry::HasComponent(Entity entity) const
{
  return IsValid(entity) && m_Registry.all_of<T>(entity.GetHandle());
}

// 3. 组件操作 - 获取 ==============================================

template<typename T> T &SceneRegistry::GetComponent(Entity entity)
{
  // 使用Assert断言，确保entity具有该组件。
  // 若无法确定，则应当使用TryGetComponent
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

// 4. 视图和查询 ============================================

template<typename... Component> bool SceneRegistry::AnyOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.any_of<Component...>(entity.GetHandle());
}

template<typename... Component> bool SceneRegistry::AllOf(Entity entity) const
{
  return IsValid(entity) && m_Registry.all_of<Component...>(entity.GetHandle());
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

  // 使用entt::registry::view方法，
  // 获取符合类型要求的Component列表
  auto view = m_Registry.view<Component...>();

  entities.reserve(view.size_hint());

  for (auto entity : view) {
    if (m_Registry.valid(entity)) {
      entities.emplace_back(m_Scene, entity);
    }
  }

  return entities;
}

// 6. 组件事件回调相关 ===============================================

template<typename T>
void SceneRegistry::RegisterCallbackComponentConstruct(ComponentCallback callback)
{
  // 编译时检查，确保注册使用的模板为Component的子类
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  // 根据typeid构建key-value对，存入哈希表
  const std::type_index type = typeid(T);
  m_ConstructCallbacks[type] = callback;

  // 连接到EnTT的回调系统
  m_Registry.on_construct<T>().template connect<&SceneRegistry::InvokeConstruct<T>>(this);
}

template<typename T>
void SceneRegistry::RegisterCallbackComponentUpdate(ComponentCallback callback)
{
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  const std::type_index type = typeid(T);
  m_UpdateCallbacks[type] = callback;

  m_Registry.on_update<T>().template connect<&SceneRegistry::InvokeUpdate<T>>(this);
}

template<typename T>
void SceneRegistry::RegisterCallbackComponentDestroy(ComponentCallback callback)
{
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

  const std::type_index type = typeid(T);
  m_DestroyCallbacks[type] = callback;

  m_Registry.on_destroy<T>().template connect<&SceneRegistry::InvokeDestroy<T>>(this);
}

template<typename T> void SceneRegistry::InvokeConstruct(Entity entity, T &component)
{
  // 以typeid作为key查表
  const std::type_index type = typeid(T);
  if (auto it = m_ConstructCallbacks.find(type); it != m_ConstructCallbacks.end()) {
    // it->second类型为函数指针
    // std::function<void(Entity, Component &)>
    // 此处可以直接运行该函数
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

// 7. 实体事件回调相关 ===============================================

size_t SceneRegistry::RegisterCallbackEntityCreated(EntityCallback callback, int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.createdCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.createdCallbacks;
  SortCallbackList(m_EntityCallbacks.createdCallbacks);
  return id;
}

size_t SceneRegistry::RegisterCallbackEntityPreDestroyed(EntityCallback callback, int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.preDestroyCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.preDestroyCallbacks;
  SortCallbackList(m_EntityCallbacks.preDestroyCallbacks);
  return id;
}

size_t SceneRegistry::RegisterCallbackEntityPostDestroyed(EntityCallback callback, int priority)
{
  const size_t id = m_NextEntityCallbackID++;
  m_EntityCallbacks.postDestroyCallbacks.push_back({std::move(callback), priority, id});
  m_EntityCallbacks.entityCallbackMap[id] = &m_EntityCallbacks.postDestroyCallbacks;
  SortCallbackList(m_EntityCallbacks.postDestroyCallbacks);
  return id;
}

void SceneRegistry::UnregisterCallbackEntity(size_t callbackId)
{
  auto it = m_EntityCallbacks.entityCallbackMap.find(callbackId);
  if (it != m_EntityCallbacks.entityCallbackMap.end()) {
    auto &callbacks = *it->second;
    // 根据remove_if返回的迭代器，
    // 从callbacks列表中，
    // 移除所有id等于callbackId的回调函数。
    callbacks.erase(
        std::remove_if(callbacks.begin(),
                       callbacks.end(),
                       [callbackId](const auto &wrapper) { return wrapper.id == callbackId; }),
        callbacks.end());
    m_EntityCallbacks.entityCallbackMap.erase(it);
  }
}

void SceneRegistry::UnregisterCallbackEntity() {
  m_EntityCallbacks.createdCallbacks.clear();
  m_EntityCallbacks.preDestroyCallbacks.clear();
  m_EntityCallbacks.postDestroyCallbacks.clear();
  m_EntityCallbacks.entityCallbackMap.clear();
}

void SceneRegistry::ExecuteCallbacks(const std::vector<EntityCallbackWrapper> &callbacks,
                                     Entity entity)
{
  for (const auto &wrapper : callbacks) {
    if (wrapper.callback) {
      wrapper.callback(entity);
    }
  }
}

void SceneRegistry::SortCallbackList(std::vector<EntityCallbackWrapper> &callbacks)
{
  std::sort(callbacks.begin(), callbacks.end(), [](const auto &a, const auto &b) {
    return a.priority > b.priority;
  });
}
};  // namespace mite