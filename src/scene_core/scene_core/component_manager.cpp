#include "component_manager.h"
#include "component_pool.h"

namespace mite {
ComponentManager::ComponentManager()
{
  // 预留空间减少rehash
  m_ComponentPools.reserve(64);
  m_EntityComponentMap.reserve(1024);
}

ComponentManager::~ComponentManager()
{
  // 自动清理所有组件池
}

template<typename T> T &ComponentManager::AddComponent(Entity entity)
{
  static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");

  ComponentType type = ComponentType::Get<T>();

  // 获取或创建组件池
  auto &pool = GetComponentPool<T>(type);

  // 添加组件到池
  T &component = pool.Add(entity);

  // 更新实体组件映射
  m_EntityComponentMap[entity].push_back(type);

  return component;
}

template<typename T> void ComponentManager::RemoveComponent(Entity entity)
{
  ComponentType type = ComponentType::Get<T>();

  // 检查是否存在该类型的池
  auto it = m_ComponentPools.find(type);
  if (it == m_ComponentPools.end()) {
    return;
  }

  // 从池中移除
  it->second->Remove(entity);

  // 更新实体组件映射
  auto &types = m_EntityComponentMap[entity];
  types.erase(std::remove(types.begin(), types.end(), type), types.end());
}

template<typename T> bool ComponentManager::HasComponent(Entity entity) const
{
  ComponentType type = ComponentType::Get<T>();

  // 检查实体是否注册了该类型
  auto entityIt = m_EntityComponentMap.find(entity);
  if (entityIt == m_EntityComponentMap.end()) {
    return false;
  }

  const auto &types = entityIt->second;
  return std::find(types.begin(), types.end(), type) != types.end();
}

template<typename T> T &ComponentManager::GetComponent(Entity entity)
{
  ComponentType type = ComponentType::Get<T>();

  // 检查组件是否存在
  if (!HasComponent<T>(entity)) {
    throw std::runtime_error("Component not found on entity");
  }

  // 获取组件池并返回组件
  auto &pool = static_cast<ComponentPool<T> &>(*m_ComponentPools.at(type));
  return pool.Get(entity);
}

template<typename T> const T &ComponentManager::GetComponent(Entity entity) const
{
  // 常量版本调用非常量版本（避免代码重复）
  return const_cast<ComponentManager *>(this)->GetComponent<T>(entity);
}

void ComponentManager::OnEntityDestroyed(Entity entity)
{
  // 移除实体的所有组件
  auto it = m_EntityComponentMap.find(entity);
  if (it != m_EntityComponentMap.end()) {
    for (auto type : it->second) {
      m_ComponentPools[type]->Remove(entity);
    }
    m_EntityComponentMap.erase(it);
  }
}

std::vector<ComponentType> ComponentManager::GetRegisteredComponentTypes() const
{
  std::vector<ComponentType> types;
  types.reserve(m_ComponentPools.size());

  for (const auto &[type, _] : m_ComponentPools) {
    types.push_back(type);
  }

  return types;
}

template<typename T> std::vector<Entity> ComponentManager::GetEntitiesWith() const
{
  ComponentType type = ComponentType::Get<T>();

  auto it = m_ComponentPools.find(type);
  if (it == m_ComponentPools.end()) {
    return {};
  }

  return it->second->GetEntities();
}

template<typename T> IComponentPool &ComponentManager::GetComponentPool(ComponentType type)
{
  // 检查是否已存在该类型的池
  auto it = m_ComponentPools.find(type);
  if (it != m_ComponentPools.end()) {
    return *it->second;
  }

  // 创建新类型的组件池
  auto pool = std::make_unique<ComponentPool<T>>();
  auto &ref = *pool;
  m_ComponentPools.emplace(type, std::move(pool));

  return ref;
}
}  // namespace mite