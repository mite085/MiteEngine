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
  Entity entity {m_Scene, m_Registry.create()};

  // 添加基本组件，自动生成唯一ID
  auto &id = AddComponent<IDComponent>(entity);

  // 添加Tag系统，用于实体搜索和筛选
  auto &tag = AddComponent<TagComponent>(entity);
  tag.SetTag(name.empty() ? "Entity_" + id.String() : name);

  // 创建事件并发布
  EntityCreatedEvent event(entity);
  EventBus::Get().Post(event);

  return entity;
}

void SceneRegistry::DestroyEntity(Entity entity)
{
  if (IsValid(entity)) {
    // 1. 创建PreDestroyed事件并发布（立即执行）
    EntityPreDestroyedEvent event(entity);
    EventBus::Get().Post(event);

    // 2. 使用entt::registry::destroy销毁实体
    //
    // 注意：
    // 若该entity存在任何已经被
    // RegisterCallbackComponentDestroy
    // 注册的Component，会在这里触发回调，
    // 运行各个被注册的callback函数
    m_Registry.destroy(entity.GetHandle());

    // 3. 创建PostDestroyed事件并发布（可延后执行）
    EntityPostDestroyedEvent event2(entity);
    EventBus::Get().Post(event2);
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


};  // namespace mite