#include "entity.h"
#include "scene.h"
#include "scene_registry.h"
#include "scene_core_components/component_headers.h"

namespace mite {
Entity::Entity(std::weak_ptr<Scene> scene, entt::entity handle) : m_Scene(scene), m_Handle(handle)
{
}

Entity::Entity(const Entity &other)
    : m_Scene(other.m_Scene),   // weak_ptr拷贝是安全的
      m_Handle(other.m_Handle)  // entt::entity可以直接拷贝
{
}

// 组件操作实现 ==============================================

//template<typename T, typename... Args> T &Entity::AddComponent(Args &&...args)
//{
//  // Error check: Cannot add component to invalid entity!
//  assert(IsValid());
//  auto scenePtr = m_Scene.lock();
//  // Error check: Scene is expired!
//  assert(scenePtr);
//
//  // 如果已有组件，先移除再添加（确保构造新对象）
//  if (HasComponent<T>()) {
//    RemoveComponent<T>();
//  }
//
//  auto &component = scenePtr->GetRegistry().AddComponent<T>(*this, std::forward<Args>(args)...);
//  scenePtr->OnComponentAdded<T>(*this, component);
//  return component;
//}
//
//template<typename T> void Entity::RemoveComponent()
//{
//  if (!IsValid())
//    return;
//
//  if (auto scenePtr = m_Scene.lock()) {
//    if (scenePtr->GetRegistry().AnyOf<T>(*this)) {
//      auto &component = scenePtr->m_Registry.get<T>(m_Handle);
//      scenePtr->OnComponentRemoved<T>(*this, component);
//      scenePtr->m_Registry.remove<T>(m_Handle);
//
//      // 特殊处理Hierarchy组件
//      if constexpr (std::is_same<T, HierarchyComponent>::value) {
//        RemoveFromParent();
//        for (auto child : GetChildren()) {
//          child.RemoveFromParent();
//        }
//      }
//    }
//  }
//}
//
//template<typename T> bool Entity::HasComponent() const
//{
//  if (!IsValid())
//    return false;
//  if (auto scenePtr = m_Scene.lock()) {
//    return scenePtr->GetRegistry().AllOf<T>(*this);
//  }
//  return false;
//}
//
//template<typename T> T &Entity::GetComponent()
//{
//  // Error check: Entity does not have component!
//  assert(HasComponent<T>());
//  if (auto scenePtr = m_Scene.lock()) {
//    return scenePtr->GetRegistry().GetComponent<T>(*this);
//  }
//  throw std::runtime_error("Scene is expired!");
//}
//
//template<typename T> const T &Entity::GetComponent() const
//{
//  // Error check: Entity does not have component!
//  assert(HasComponent<T>());
//  if (auto scenePtr = m_Scene.lock()) {
//    return scenePtr->GetRegistry().GetComponent<T>(*this);
//  }
//  throw std::runtime_error("Scene is expired!");
//}
//
//template<typename T> T *Entity::TryGetComponent()
//{
//  if (!IsValid())
//    return nullptr;
//  if (auto scenePtr = m_Scene.lock()) {
//    return scenePtr->GetRegistry().TryGetComponent<T>(*this);
//  }
//  return nullptr;
//}
//
//template<typename T> const T *Entity::TryGetComponent() const
//{
//  if (!IsValid())
//    return nullptr;
//  if (auto scenePtr = m_Scene.lock()) {
//    return scenePtr->GetRegistry().TryGetComponent<T>(*this);
//  }
//  return nullptr;
//}

// 层次结构操作实现 ==========================================

//void Entity::SetParent(Entity parent, bool keepWorldTransform)
//{
//  if (!IsValid() || parent == *this || IsDescendantOf(parent)) {
//    return;
//  }
//
//  RemoveFromParent();
//
//  if (parent.IsValid()) {
//    auto &hierarchy = GetComponent<HierarchyComponent>();
//    hierarchy.SetParent(parent);
//
//    auto &parentHierarchy = parent.GetComponent<HierarchyComponent>();
//    parentHierarchy.AddChild(*this);
//
//    UpdateChildParentRelationship(keepWorldTransform);
//  }
//}
//
//Entity Entity::GetParent() const
//{
//  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
//    return Entity();
//  }
//
//  const auto &hierarchy = GetComponent<HierarchyComponent>();
//  if (!hierarchy.GetParent().IsValid()) {
//    return Entity();
//  }
//
//  return hierarchy.GetParent();
//}
//
//const std::vector<Entity> &Entity::GetChildren() const
//{
//  // 所有线程共享一份static const常量，减少开销(?，是否有存在意义)
//  static const std::vector<Entity> emptyChildren;
//
//  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
//    return emptyChildren;
//  }
//
//  const auto &hierarchy = GetComponent<HierarchyComponent>();
//  if (!hierarchy.GetChildren().empty()) {
//    // 确保线程安全，使用thread_local关键字
//    thread_local std::vector<Entity> children;
//    children.clear();
//    for (auto childHandle : hierarchy.GetChildren()) {
//      children.emplace_back(m_Scene, childHandle);
//    }
//    return children;
//  }
//  return emptyChildren;
//}
//
//void Entity::AddChild(Entity child, bool keepWorldTransform)
//{
//  if (child.IsValid()) {
//    child.SetParent(*this, keepWorldTransform);
//  }
//}
//
//void Entity::RemoveChild(Entity child, bool keepWorldTransform)
//{
//  if (child.IsValid() && child.GetParent() == *this) {
//    child.SetParent(Entity(), keepWorldTransform);
//  }
//}
//
//bool Entity::IsDescendantOf(Entity potentialAncestor) const
//{
//  if (!IsValid() || !potentialAncestor.IsValid()) {
//    return false;
//  }
//
//  Entity parent = GetParent();
//  while (parent.IsValid()) {
//    if (parent == potentialAncestor) {
//      return true;
//    }
//    parent = parent.GetParent();
//  }
//
//  return false;
//}

// 实体状态操作 ==============================================

//const std::string &Entity::GetID() const
//{
//  // Error check: Invalid entity has no ID!
//  assert(IsValid());
//  return GetComponent<IDComponent>().String();
//}

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

  m_Handle = entt::null;
}

entt::entity Entity::GetHandle() const
{
  return m_Handle;
}

// 操作符重载 ===============================================

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

SceneRegistry& Entity::GetSceneRegistry() const
{
  return GetScene()->GetRegistry();
}

// 私有方法 =================================================

//void Entity::UpdateChildParentRelationship(bool keepWorldTransform)
//{
//  if (keepWorldTransform) {
//    auto parent = GetParent();
//    // 检查parent实体存在性
//    if (parent.IsValid()) {
//      // 检查parent是否包含Transform component
//      if (HasComponent<TransformComponent>() && parent.HasComponent<TransformComponent>()) {
//        auto &this_transform = GetComponent<TransformComponent>();
//        auto &parent_transform = parent.GetComponent<TransformComponent>();
//
//        // 保存当前世界变换
//        glm::mat4 worldMatrix = this_transform.GetWorldMatrix();
//
//        // 计算相对于新父级的局部变换
//        glm::mat4 parentWorldMatrix = parent_transform.GetWorldMatrix();
//        glm::mat4 newLocalMatrix = glm::inverse(parentWorldMatrix) * worldMatrix;
//
//        // 分解矩阵应用到局部变换
//        glm::vec3 scale;
//        glm::quat rotation;
//        glm::vec3 translation;
//        glm::vec3 skew;
//        glm::vec4 perspective;
//
//        if (glm::decompose(newLocalMatrix, scale, rotation, translation, skew, perspective)) {
//          // 应用新的局部变换
//          this_transform.SetLocalPosition(translation);
//          this_transform.SetLocalRotation(rotation);
//          this_transform.SetLocalScale(scale);
//        }
//        else {
//          // 分解失败，保持原样(Entity为轻量级组件基类，并不包含独立的logger)
//          LOG_WARN("Failed to decompose matrix when updating parent-child relationship");
//        }
//      }
//      else {
//        // this或parent实体无Transform组件，无需改变
//        return;
//      }
//    }
//    else {
//      // 无parent实体，无需改变
//      return;
//    }
//  }
//}
//
//void Entity::RemoveFromParent()
//{
//  if (!IsValid() || !HasComponent<HierarchyComponent>()) {
//    return;
//  }
//
//  auto &hierarchy = GetComponent<HierarchyComponent>();
//  if (hierarchy.GetParent().IsValid()) {
//    if (auto parent = GetParent(); parent.IsValid()) {
//      auto &parentHierarchy = parent.GetComponent<HierarchyComponent>();
//      parentHierarchy.RemoveChild(*this);
//    }
//    hierarchy.SetParent(Entity{});
//  }
//}
};  // namespace mite