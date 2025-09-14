#include "scene_registry.h"
#include "scene_core_components/id_component.h"
#include "scene_core_components/tag_component.h"

namespace mite {
SceneRegistry::SceneRegistry() : m_ComponentEventPublisher() {}

SceneRegistry::~SceneRegistry()
{
  Clear();
}

ComponentEventPublisher &SceneRegistry::GetEventPublisher()
{
  return m_ComponentEventPublisher;
}

// 1. 实体管理 ===================================================

Entity SceneRegistry::CreateEntity(const std::string& name)
{
  // 创建实体
  Entity entity = Entity::CreateEntity();

  // 添加ID组件，自动生成唯一ID
  auto &id = AddComponent<IDComponent>(entity);

  // 添加Tag组件，用于实体搜索和筛选
  auto &tag = AddComponent<TagComponent>(entity);
  tag.SetTag(name.empty() ? "Entity_" + id.String() : name);

  // 创建事件并发布
  EntityCreatedEvent event(entity);
  EventBus::Publish<EntityCreatedEvent>(event);

  return entity;
}

void SceneRegistry::DestroyEntity(Entity entity)
{
  if (!IsValid(entity)) {
    return;
  }

  // 移除所有组件
  std::unique_lock lock(m_ComponentMutex);
  for (auto &pair : m_Components) {
    pair.second.erase(entity);
  }

  // 标记实体为无效
  entity.Destroy();
}

bool SceneRegistry::IsValid(Entity entity) const
{
  return entity.IsValid();
}

void SceneRegistry::Clear()
{
  std::unique_lock lock(m_ComponentMutex);
  m_Components.clear();
}

std::vector<Entity> SceneRegistry::GetAllEntities()
{
  std::shared_lock lock(m_ComponentMutex);

  std::vector<Entity> entities;
  if (!m_Components.empty()) {
    // 使用第一个组件类型的实体列表作为基准
    auto &firstComponentMap = m_Components.begin()->second;
    entities.reserve(firstComponentMap.size());

    for (const auto &pair : firstComponentMap) {
      if (IsValid(pair.first)) {
        entities.push_back(pair.first);
      }
    }
  }
  return entities;
}


};  // namespace mite