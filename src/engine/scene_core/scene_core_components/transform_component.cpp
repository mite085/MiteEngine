#include "transform_component.h"

namespace mite {
TransformComponent::TransformComponent() : ComponentTraits(), m_Transform() {}

TransformComponent::TransformComponent(const glm::vec3 position,
                                       const glm::vec3 rotation,
                                       const glm::vec3 scale,
                                       const Transform::EulerOrder order)
    : ComponentTraits(), m_Transform(position, rotation, scale, order)
{
}

TransformComponent::TransformComponent(const glm::mat4 &matrix, const Transform::EulerOrder order)
    : ComponentTraits(), m_Transform(matrix, order)
{
}

void TransformComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  // 由于Transform内部的DirtyFlag未确保清除，在此步骤执行CleanDirty操作
  m_Transform.CleanDirty();

  // 发布事件通知SceneNode系统更新变换
  EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
}
// 位置相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalPosition() const
{
  return m_Transform.GetPosition();
}

void TransformComponent::SetLocalPosition(const glm::vec3 &position)
{
  if (m_Transform.GetPosition() != position) {
    m_Transform.SetPosition(position);
    MarkDirty();
  }
}
// 旋转相关方法 ==============================================

Transform::EulerOrder TransformComponent::GetRotationOrder() const
{
  return m_Transform.GetRotationOrder();
}
glm::vec3 TransformComponent::GetLocalRotationEuler() const
{
  return m_Transform.GetRotationEuler();
}
glm::quat TransformComponent::GetLocalRotationQuat() const
{
  return m_Transform.GetRotationQuat();
}

void TransformComponent::SetLocalRotation(const glm::vec3 &rotation)
{
  if (m_Transform.GetRotationEuler() != rotation) {
    m_Transform.SetRotationEuler(rotation);
    MarkDirty();
  }
}
void TransformComponent::SetLocalRotation(float x, float y, float z)
{
  SetLocalRotation(glm::vec3{x, y, z});
}
void TransformComponent::SetLocalRotationQuat(const glm::quat &rotation)
{
  if (m_Transform.GetRotationQuat() != rotation) {
    m_Transform.SetRotationQuat(rotation);
    MarkDirty();
  }
}
void TransformComponent::LookAt(const glm::vec3 &target, const glm::vec3 &up)
{
  m_Transform.LookAt(target, up);
  MarkDirty();
}

// 缩放相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalScale() const
{
  return m_Transform.GetScale();
}
void TransformComponent::SetLocalScale(const glm::vec3 &scale)
{
  if (m_Transform.GetScale() != scale) {
    m_Transform.SetScale(scale);
    MarkDirty();
  }
}
void TransformComponent::SetLocalScale(float scale)
{
  SetLocalScale(glm::vec3{scale, scale, scale});
}


// 矩阵相关方法 ==============================================

glm::mat4 TransformComponent::GetLocalMatrix() const
{
  return m_Transform.GetLocalMatrix();
}
void TransformComponent::SetLocalMatrix(const glm::mat4 &matrix)
{
  m_Transform.SetLocalMatrix(matrix);
  MarkDirty();
}
glm::mat4 TransformComponent::CreateViewMatrix() const
{
  return m_Transform.GetViewMatrix();
}


// 方向向量 ==============================================

glm::vec3 TransformComponent::GetForward() const
{
  return m_Transform.GetForward();
}

glm::vec3 TransformComponent::GetUp() const
{
  return m_Transform.GetUp();
}

glm::vec3 TransformComponent::GetRight() const
{
  return m_Transform.GetRight();
}

glm::vec3 TransformComponent::GetConstrainedUp(const glm::vec3 &worldUp) const
{
  return m_Transform.GetConstrainedUp();
}

glm::vec3 TransformComponent::GetConstrainedRight(const glm::vec3 &worldUp) const
{
  return m_Transform.GetConstrainedRight();
}

glm::vec3 TransformComponent::GetConstrainedForward(const glm::vec3 &worldUp) const
{
  return m_Transform.GetConstrainedForward();
}

// 组件接口实现 ==========================================

std::vector<std::type_index> TransformComponent::GetDependencies() const
{
  return {};
}

bool TransformComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);
  // 序列化留空，等待序列化模块
  return !output.fail();
}
bool TransformComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);
  // 反序列化留空，等待序列化模块
  return !input.fail();
}

// ==================== 组件系统实现 ====================

std::vector<std::type_index> TransformComponentSystem::GetSystemDependencies() const
{
  return {};  // 依赖层级信息
}

bool TransformComponentSystem::OnComponentAdded(ComponentAddedEvent<TransformComponent> &e)
{
  Register(&e.GetComponent());

  // 不应当标记事件已处理，继续传播给SceneGraph的TransformSceneNodeSystem
  // e.Handled();
  return e.handled;
}

/**
 * @brief 处理组件移除事件
 */
bool TransformComponentSystem::OnComponentRemoved(ComponentRemovedEvent<TransformComponent> &e)
{
  Unregister(&e.GetComponent());

  // 不应当标记事件已处理，继续传播给SceneGraph的TransformSceneNodeSystem
  // e.Handled();
  return e.handled;
}

};  // namespace mite