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

// 4. 视图和查询 ============================================

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