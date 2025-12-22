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

Entity SceneRegistry::CreateEntity(const std::string &name, Entity parent)
{
  // 创建实体
  Entity entity = Entity::CreateEntity(name);

  // 添加ID组件，自动生成唯一ID
  auto &id = AddComponent<IDComponent>(entity);

  // 添加Tag组件，用于实体搜索和筛选
  auto &tag = AddComponent<TagComponent>(entity);
  tag.SetTag(name.empty() ? "Entity_" + id.String() : name);

  // 创建事件并发布（此处不检查Parent的可用性，空实体对应无Parent的根节点语义）
  EventBus::Publish<EntityCreatedEvent>(entity, parent);

  return entity;
}

void SceneRegistry::DestroyEntity(Entity entity)
{
  if (!entity.IsValid()) {
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
      if (pair.first.IsValid()) {
        entities.push_back(pair.first);
      }
    }
  }
  return entities;
}


};  // namespace mite