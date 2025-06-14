#include "entity.h"
#include "scene.h"
#include "scene_core_components/component_headers.h"

namespace mite {
Entity::Entity(std::weak_ptr<Scene> scene, entt::entity handle) : m_Scene(scene), m_Handle(handle)
{
  // ȷ��ʵ���б������
  if (auto scenePtr = m_Scene.lock()) {
    if (!scenePtr->GetRegistry().any_of<IDComponent>(m_Handle)) {
      scenePtr->GetRegistry().emplace<IDComponent>(m_Handle);
    }
    if (!scenePtr->GetRegistry().any_of<HierarchyComponent>(m_Handle)) {
      scenePtr->GetRegistry().emplace<HierarchyComponent>(m_Handle);
    }
  }
}

// �������ʵ�� ==============================================

template<typename T, typename... Args> T &Entity::AddComponent(Args &&...args)
{
  // Error check: Cannot add component to invalid entity!
  assert(IsValid());
  auto scenePtr = m_Scene.lock();
  // Error check: Scene is expired!
  assert(scenePtr);

  // ���������������Ƴ�����ӣ�ȷ�������¶���
  if (HasComponent<T>()) {
    RemoveComponent<T>();
  }

  auto &component = scenePtr->Reg().emplace<T>(m_Handle, std::forward<Args>(args)...);
  scenePtr->OnComponentAdded<T>(*this, component);
  return component;
}

template<typename T> void Entity::RemoveComponent()
{
  if (!IsValid())
    return;

  if (auto scenePtr = m_Scene.lock()) {
    if (scenePtr->m_Registry.any_of<T>(m_Handle)) {
      auto &component = scenePtr->m_Registry.get<T>(m_Handle);
      scenePtr->OnComponentRemoved<T>(*this, component);
      scenePtr->m_Registry.remove<T>(m_Handle);

      // ���⴦��Hierarchy���
      if constexpr (std::is_same_v<T, HierarchyComponent>) {
        RemoveFromParent();
        for (auto child : GetChildren()) {
          child.RemoveFromParent();
        }
      }
    }
  }
}

template<typename T> bool Entity::HasComponent() const
{
  if (!IsValid())
    return false;
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->m_Registry.all_of<T>(m_Handle);
  }
  return false;
}

template<typename T> T &Entity::GetComponent()
{
  // Error check: Entity does not have component!
  assert(HasComponent<T>());
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->m_Registry.get<T>(m_Handle);
  }
  throw std::runtime_error("Scene is expired!");
}

template<typename T> const T &Entity::GetComponent() const
{
  // Error check: Entity does not have component!
  assert(HasComponent<T>());
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->m_Registry.get<T>(m_Handle);
  }
  throw std::runtime_error("Scene is expired!");
}

template<typename T> T *Entity::TryGetComponent()
{
  if (!IsValid())
    return nullptr;
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->m_Registry.try_get<T>(m_Handle);
  }
  return nullptr;
}

template<typename T> const T *Entity::TryGetComponent() const
{
  if (!IsValid())
    return nullptr;
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->m_Registry.try_get<T>(m_Handle);
  }
  return nullptr;
}

// ��νṹ����ʵ�� ==========================================

void Entity::SetParent(Entity parent, bool keepWorldTransform)
{
  if (!IsValid() || parent == *this || IsDescendantOf(parent)) {
    return;
  }

  RemoveFromParent();

  if (parent.IsValid()) {
    auto &hierarchy = GetComponent<HierarchyComponent>();
    hierarchy.SetParent(parent.m_Handle);

    auto &parentHierarchy = parent.GetComponent<HierarchyComponent>();
    parentHierarchy.AddChild(m_Handle);

    UpdateChildParentRelationship(*this, keepWorldTransform);
  }
}

Entity Entity::GetParent() const
{
  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
    return Entity();
  }

  const auto &hierarchy = GetComponent<HierarchyComponent>();
  if (hierarchy.GetParent() == entt::null) {
    return Entity();
  }

  if (auto scenePtr = m_Scene.lock()) {
    return Entity(scenePtr, hierarchy.GetParent());
  }
  return Entity();
}

const std::vector<Entity> &Entity::GetChildren() const
{
  static const std::vector<Entity> emptyChildren;

  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
    return emptyChildren;
  }

  if (auto scenePtr = m_Scene.lock()) {
    const auto &hierarchy = GetComponent<HierarchyComponent>();
    if (!hierarchy.GetChildren().empty()) {
      thread_local std::vector<Entity> children;
      children.clear();
      for (auto childHandle : hierarchy.GetChildren()) {
        children.emplace_back(scenePtr, childHandle);
      }
      return children;
    }
  }

  return emptyChildren;
}

void Entity::AddChild(Entity child, bool keepWorldTransform)
{
  if (child.IsValid()) {
    child.SetParent(*this, keepWorldTransform);
  }
}

void Entity::RemoveChild(Entity child, bool keepWorldTransform)
{
  if (child.IsValid() && child.GetParent() == *this) {
    child.SetParent(Entity(), keepWorldTransform);
  }
}

bool Entity::IsDescendantOf(Entity potentialAncestor) const
{
  if (!IsValid() || !potentialAncestor.IsValid()) {
    return false;
  }

  Entity parent = GetParent();
  while (parent.IsValid()) {
    if (parent == potentialAncestor) {
      return true;
    }
    parent = parent.GetParent();
  }

  return false;
}

// ʵ��״̬���� ==============================================

const std::string &Entity::GetID() const
{
  // Error check: Invalid entity has no ID!
  assert(IsValid());
  return GetComponent<IDComponent>().String();
}

bool Entity::IsValid() const
{
  if (m_Handle == entt::null)
    return false;
  if (auto scenePtr = m_Scene.lock()) {
    return scenePtr->IsValid(*this);
  }
  return false;
}

void Entity::Destroy()
{
  if (!IsValid())
    return;

  if (auto scenePtr = m_Scene.lock()) {
    // ������������ʵ��
    auto children = GetChildren();  // ����һ�ݣ���Ϊԭ����������ٹ������޸�
    for (auto child : children) {
      child.Destroy();
    }

    // �Ӹ����Ƴ�
    RemoveFromParent();

    // ���ʵ��Ϊ����״̬
    scenePtr->GetRegistry().destroy(m_Handle);
    m_Handle = entt::null;
  }
}

// ���������� ===============================================

bool Entity::operator==(const Entity &other) const
{
  return m_Handle == other.m_Handle && m_Scene.lock() == other.m_Scene.lock();
}

bool Entity::operator!=(const Entity &other) const
{
  return !(*this == other);
}

Entity::operator bool() const
{
  return IsValid();
}

std::shared_ptr<Scene> Entity::GetScene() const
{
  return m_Scene.lock();
}

// ˽�з��� =================================================

void Entity::UpdateChildParentRelationship(Entity child, bool keepWorldTransform)
{
  // TODO: ���������ӱ�������任���߼�
  // ʵ��ʵ����ҪTransformSystem��֧��
  if (keepWorldTransform) {
    // TODO: ���㲢���¾ֲ��任�Ա��ֵ�ǰ����任
  }
}

void Entity::RemoveFromParent()
{
  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
    return;
  }

  auto &hierarchy = GetComponent<HierarchyComponent>();
  if (hierarchy.GetParent() != entt::null) {
    if (auto parent = GetParent(); parent.IsValid()) {
      auto &parentHierarchy = parent.GetComponent<HierarchyComponent>();
      parentHierarchy.RemoveChild(m_Handle);
    }
    hierarchy.SetParent(entt::null);
  }
}
};  // namespace mite