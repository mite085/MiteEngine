#include "Entity.h"
#include "Component.h"
#include "Scene.h"

namespace mite {
Entity::Entity(Scene *scene, EntityID id)
    : m_Scene(scene), m_ID(id), m_IsActive(true), m_Name("Entity_" + std::to_string(id))
{
}

Entity::~Entity()
{
  // 移除所有组件
  m_Components.clear();

  // 从父实体中移除自己
  if (m_Parent) {
    m_Parent->RemoveChild(this);
  }

  // 移除所有子实体的父引用
  for (auto child : m_Children) {
    child->m_Parent = nullptr;
  }
}

EntityID Entity::GetID() const
{
  return m_ID;
}

Scene *Entity::GetScene() const
{
  return m_Scene;
}

bool Entity::IsActive() const
{
  return m_IsActive;
}

void Entity::SetActive(bool active)
{
  m_IsActive = active;
}

const std::string &Entity::GetName() const
{
  return m_Name;
}

void Entity::SetName(const std::string &name)
{
  m_Name = name;
}

// ==================== 组件管理 ====================

template<typename T, typename... Args> T *Entity::AddComponent(Args &&...args)
{
  static_assert(std::is_base_of<Component, T>::value, "T must be a derived class of Component");

  auto typeIndex = std::type_index(typeid(T));

  // 如果已有该类型组件，先移除
  if (m_Components.find(typeIndex) != m_Components.end()) {
    RemoveComponent<T>();
  }

  // 创建新组件
  auto component = std::make_unique<T>(std::forward<Args>(args)...);
  component->m_Owner = this;

  // 存储组件并获取原始指针
  T *rawPtr = component.get();
  m_Components[typeIndex] = std::move(component);

  // 通知场景组件添加
  if (m_Scene) {
    m_Scene->OnComponentAdded(this, rawPtr);
  }

  return rawPtr;
}

template<typename T> void Entity::RemoveComponent()
{
  static_assert(std::is_base_of<Component, T>::value, "T must be a derived class of Component");

  auto typeIndex = std::type_index(typeid(T));
  auto it = m_Components.find(typeIndex);

  if (it != m_Components.end()) {
    // 通知场景组件将被移除
    if (m_Scene) {
      m_Scene->OnComponentRemoved(this, it->second.get());
    }

    // 移除组件
    m_Components.erase(it);
  }
}

template<typename T> bool Entity::HasComponent() const
{
  static_assert(std::is_base_of<Component, T>::value, "T must be a derived class of Component");

  return m_Components.find(std::type_index(typeid(T))) != m_Components.end();
}

template<typename T> T *Entity::GetComponent()
{
  static_assert(std::is_base_of<Component, T>::value, "T must be a derived class of Component");

  auto it = m_Components.find(std::type_index(typeid(T)));
  return it != m_Components.end() ? static_cast<T *>(it->second.get()) : nullptr;
}

template<typename T> const T *Entity::GetComponent() const
{
  static_assert(std::is_base_of<Component, T>::value, "T must be a derived class of Component");

  auto it = m_Components.find(std::type_index(typeid(T)));
  return it != m_Components.end() ? static_cast<const T *>(it->second.get()) : nullptr;
}

// ==================== 父子关系管理 ====================

void Entity::SetParent(Entity *parent)
{
  if (parent == m_Parent) {
    return;
  }

  // 从当前父节点移除
  if (m_Parent) {
    m_Parent->RemoveChild(this);
  }

  // 设置新父节点
  m_Parent = parent;

  // 添加到新父节点的子列表
  if (m_Parent) {
    m_Parent->AddChild(this);
  }
}

void Entity::AddChild(Entity *child)
{
  if (!child || child == this) {
    return;
  }

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  // 确保不会形成循环
  if (child->IsAncestorOf(this)) {
    // 错误处理 - 不能形成循环引用
    return;
  }

  m_Children.push_back(child);
  child->m_Parent = this;
}

void Entity::RemoveChild(Entity *child)
{
  if (!child) {
    return;
  }

  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
    child->m_Parent = nullptr;
  }
}

bool Entity::IsAncestorOf(const Entity *entity) const
{
  if (!entity) {
    return false;
  }

  Entity *parent = entity->m_Parent;
  while (parent) {
    if (parent == this) {
      return true;
    }
    parent = parent->m_Parent;
  }

  return false;
}

bool Entity::IsDescendantOf(const Entity *entity) const
{
  return entity ? entity->IsAncestorOf(this) : false;
}
}  // namespace mite