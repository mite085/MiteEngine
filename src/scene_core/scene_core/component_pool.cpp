#include "component_pool.h"

namespace mite {
template<typename T> T &ComponentPool<T>::Add(Entity entity)
{
  // 检查是否已存在
  if (m_EntityToIndex.count(entity) > 0) {
    throw std::runtime_error("Entity already has this component");
  }

  // 记录新索引
  const size_t index = m_Components.size();
  m_EntityToIndex[entity] = index;

  // 在尾部添加新元素
  m_Entities.push_back(entity);
  m_Components.emplace_back();  // 默认构造组件

  return m_Components.back();
}

template<typename T> void ComponentPool<T>::Remove(Entity entity)
{
  // 查找实体索引
  auto it = m_EntityToIndex.find(entity);
  if (it == m_EntityToIndex.end()) {
    return;
  }

  const size_t index = it->second;
  const Entity lastEntity = m_Entities.back();

  // 将要删除的元素与尾部元素交换
  if (index < m_Components.size() - 1) {
    std::swap(m_Entities[index], m_Entities.back());
    std::swap(m_Components[index], m_Components.back());

    // 更新交换后元素的索引
    m_EntityToIndex[lastEntity] = index;
  }

  // 移除尾部元素
  m_Entities.pop_back();
  m_Components.pop_back();
  m_EntityToIndex.erase(it);
}

template<typename T> bool ComponentPool<T>::Has(Entity entity) const
{
  return m_EntityToIndex.count(entity) > 0;
}

template<typename T> T &ComponentPool<T>::Get(Entity entity)
{
  try {
    return m_Components.at(m_EntityToIndex.at(entity));
  }
  catch (const std::out_of_range &) {
    throw std::out_of_range("Component not found for entity");
  }
}

template<typename T> const T &ComponentPool<T>::Get(Entity entity) const
{
  // 调用非const版本避免代码重复
  return const_cast<ComponentPool<T> *>(this)->Get(entity);
}

template<typename T> std::vector<Entity> ComponentPool<T>::GetEntities() const
{
  return m_Entities;  // 返回副本
}

template<typename T> size_t ComponentPool<T>::Size() const
{
  return m_Components.size();
}
};
