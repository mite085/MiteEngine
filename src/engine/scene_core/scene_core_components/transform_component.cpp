#include "transform_component.h"

namespace mite {
TransformComponent::TransformComponent() : SnapshotComponentTraits(), m_Transform() {}

TransformComponent::TransformComponent(const glm::vec3 position,
                                       const glm::vec3 rotation,
                                       const glm::vec3 scale,
                                       const Transform::EulerOrder order)
    : SnapshotComponentTraits(), m_Transform(position, rotation, scale, order)
{
}

TransformComponent::TransformComponent(const glm::mat4 &matrix, const Transform::EulerOrder order)
    : SnapshotComponentTraits(), m_Transform(matrix, order)
{
}
std::vector<std::type_index> TransformComponent::GetDependencies() const
{
  return {};
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

    // 虽然 m_Transform.SetPosition 仅仅设置了m_MatrixDirty = true;
    // 但一旦其他模块获取到事件之后，使用m_Transform.Get()获取值进行更新时
    // 会自动执行UpdateMatrix，清除脏标记（Transform内部的Dirty自洽），
    // 所以可以在Set之后立即发布事件，无需担心Transform内部数据迟滞的问题。
    EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
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
    EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
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
    EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
  }
}
void TransformComponent::LookAt(const glm::vec3 &target, const glm::vec3 &up)
{
  m_Transform.LookAt(target, up);
  EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
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
    EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
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
  EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
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

// ==================== 快照接口实现 ====================
Transform TransformComponent::GetSnapshotData() const
{
  return m_Transform;
}

void TransformComponent::SetSnapshotData(const Transform &data)
{
  m_Transform = data;
  // 发布更新事件
  EventBus::Publish<TransformUpdatedEvent>(TransformUpdatedEvent(GetEntity(), *this));
}


};  // namespace mite