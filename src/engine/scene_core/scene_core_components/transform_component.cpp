#include "transform_component.h"

namespace mite {
TransformComponent::TransformComponent()
    : SnapshotComponentTraits(), m_Transform(std::make_shared<Transform>()) {}

TransformComponent::TransformComponent(const glm::vec3 position,
                                       const glm::vec3 rotation,
                                       const glm::vec3 scale,
                                       const Transform::EulerOrder order)
    : SnapshotComponentTraits(),
      m_Transform(
          std::make_shared<Transform>(position, rotation, scale, order)) {}

TransformComponent::TransformComponent(const glm::mat4 &matrix,
                                       const Transform::EulerOrder order)
    : SnapshotComponentTraits(),
      m_Transform(std::make_shared<Transform>(matrix, order)) {}
std::vector<std::type_index> TransformComponent::GetDependencies() const {
  return {};
}

// ==================== 数据操作 ====================
const Transform &TransformComponent::GetLocalTransform() const {
  return *m_Transform;
}
void TransformComponent::SetLocalTransform(const Transform &transform) {
  if (m_Transform) {
    *m_Transform = transform;
    EventBus::Publish<TransformUpdatedEvent>(GetEntity(), *this);
  }
}
void TransformComponent::SetLocalTransform(
    std::function<void(Transform &)> transformOperator) {
  if (m_Transform) {
    transformOperator(*m_Transform);

    // 虽然 m_Transform.SetPosition、SetRotation等仅仅设置了m_MatrixDirty =
    // true; 但一旦其他模块获取到事件之后，使用m_Transform.Get()获取值进行更新时
    // 会自动执行UpdateMatrix，清除脏标记（Transform内部的Dirty自洽），
    // 所以可以在Set之后立即发布事件，无需担心Transform内部数据迟滞的问题。
    EventBus::Publish<TransformUpdatedEvent>(GetEntity(), *this);
  }
}

// ==================== 序列化接口 ====================
bool TransformComponent::Serialize(std::ostream &output) const {
  Component::Serialize(output);
  // 序列化留空，等待序列化模块
  return !output.fail();
}
bool TransformComponent::Deserialize(std::istream &input) {
  Component::Deserialize(input);
  // 反序列化留空，等待序列化模块
  return !input.fail();
}

// ==================== 快照接口实现 ====================
const Transform &TransformComponent::GetSnapshotData() const {
  return *m_Transform;
}

void TransformComponent::SetSnapshotData(const Transform &data) {
  *m_Transform = data;
  // 发布更新事件
  EventBus::Publish<TransformUpdatedEvent>(GetEntity(), *this);
}
};  // namespace mite