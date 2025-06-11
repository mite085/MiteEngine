#include "component_manager.h"
#include "component_pool.h"

namespace mite {
ComponentManager::ComponentManager()
{
  // Ԥ���ռ����rehash
  m_ComponentPools.reserve(64);
  m_EntityComponentMap.reserve(1024);
}

ComponentManager::~ComponentManager()
{
  // �Զ��������������
}

template<typename T> T &ComponentManager::AddComponent(Entity entity)
{
  static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");

  ComponentType type = ComponentType::Get<T>();

  // ��ȡ�򴴽������
  auto &pool = GetComponentPool<T>(type);

  // ����������
  T &component = pool.Add(entity);

  // ����ʵ�����ӳ��
  m_EntityComponentMap[entity].push_back(type);

  return component;
}

template<typename T> void ComponentManager::RemoveComponent(Entity entity)
{
  ComponentType type = ComponentType::Get<T>();

  // ����Ƿ���ڸ����͵ĳ�
  auto it = m_ComponentPools.find(type);
  if (it == m_ComponentPools.end()) {
    return;
  }

  // �ӳ����Ƴ�
  it->second->Remove(entity);

  // ����ʵ�����ӳ��
  auto &types = m_EntityComponentMap[entity];
  types.erase(std::remove(types.begin(), types.end(), type), types.end());
}

template<typename T> bool ComponentManager::HasComponent(Entity entity) const
{
  ComponentType type = ComponentType::Get<T>();

  // ���ʵ���Ƿ�ע���˸�����
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

  // �������Ƿ����
  if (!HasComponent<T>(entity)) {
    throw std::runtime_error("Component not found on entity");
  }

  // ��ȡ����ز��������
  auto &pool = static_cast<ComponentPool<T> &>(*m_ComponentPools.at(type));
  return pool.Get(entity);
}

template<typename T> const T &ComponentManager::GetComponent(Entity entity) const
{
  // �����汾���÷ǳ����汾����������ظ���
  return const_cast<ComponentManager *>(this)->GetComponent<T>(entity);
}

void ComponentManager::OnEntityDestroyed(Entity entity)
{
  // �Ƴ�ʵ����������
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
  // ����Ƿ��Ѵ��ڸ����͵ĳ�
  auto it = m_ComponentPools.find(type);
  if (it != m_ComponentPools.end()) {
    return *it->second;
  }

  // ���������͵������
  auto pool = std::make_unique<ComponentPool<T>>();
  auto &ref = *pool;
  m_ComponentPools.emplace(type, std::move(pool));

  return ref;
}
}  // namespace mite