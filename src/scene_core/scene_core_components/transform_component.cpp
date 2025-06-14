#include "transform_component.h"
#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
// 静态初始化
constexpr float EPSILON = 0.00001f;

TransformComponent::TransformComponent(std::weak_ptr<Entity> owner) : ComponentTraits(owner) {}

TransformComponent::TransformComponent(std::weak_ptr<Entity> owner,
                                       const glm::vec3 &position,
                                       const glm::quat &rotation,
                                       const glm::vec3 &scale)
    : ComponentTraits(owner), m_Position(position), m_Rotation(rotation), m_Scale(scale)
{
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
}

TransformComponent::TransformComponent(std::weak_ptr<Entity> owner, const glm::mat4 &matrix)
    : ComponentTraits(owner)
{
  DecomposeMatrix(matrix);
}

// 位置相关方法 ==============================================

void TransformComponent::SetLocalPosition(const glm::vec3 &position)
{
  if (m_Position != position) {
    m_Position = position;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    SetDirty();
  }
}

glm::vec3 TransformComponent::GetWorldPosition() const
{
  const glm::mat4 &worldMat = GetWorldMatrix();
  return glm::vec3(worldMat[3]);
}

void TransformComponent::SetWorldPosition(const glm::vec3 &position)
{
  if (GetOwner().lock()->HasComponent<HierarchyComponent>()) {
    auto &hierarchy = GetOwner().lock()->GetComponent<HierarchyComponent>();
    if (hierarchy.GetParent() != entt::null) {
      // 如果有父节点，转换为局部位置
      TransformComponent &parentTransform =
          hierarchy.GetParent()->GetComponent<TransformComponent>();
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix();
      glm::mat4 inverseParent = glm::inverse(parentWorldMat);
      glm::vec4 localPos = inverseParent * glm::vec4(position, 1.0f);
      SetLocalPosition(glm::vec3(localPos));
      return;
    }
  }
  SetLocalPosition(position);
}

void TransformComponent::Translate(const glm::vec3 &translation)
{
  m_Position += translation;
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  SetDirty();
}

// 旋转相关方法 ==============================================

void TransformComponent::SetLocalRotation(const glm::quat &rotation)
{
  if (m_Rotation != rotation) {
    m_Rotation = rotation;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    SetDirty();
  }
}

glm::vec3 TransformComponent::GetLocalEulerAngles() const
{
  return glm::eulerAngles(m_Rotation);
}

void TransformComponent::SetLocalEulerAngles(const glm::vec3 &eulerAngles)
{
  SetLocalRotation(glm::quat(eulerAngles));
}

glm::quat TransformComponent::GetWorldRotation() const
{
  const glm::mat4 &worldMat = GetWorldMatrix();
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(worldMat, scale, rotation, translation, skew, perspective);
  return rotation;
}

void TransformComponent::Rotate(const glm::quat &rotation)
{
  m_Rotation = rotation * m_Rotation;
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  SetDirty();
}

void TransformComponent::LookAt(const glm::vec3 &target, const glm::vec3 &up)
{
  const glm::vec3 position = GetWorldPosition();
  const glm::mat4 lookAtMat = glm::lookAt(position, target, up);
  const glm::quat rotation = glm::quat_cast(glm::inverse(lookAtMat));

  if (GetOwner().lock()->HasComponent<HierarchyComponent>()) {
    auto &hierarchy = GetOwner().lock()->GetComponent<HierarchyComponent>();
    if (hierarchy.GetParent() != entt::null) {
      // 转换为局部旋转
      TransformComponent &parentTransform =
          hierarchy.GetParent()->GetComponent<TransformComponent>();
      glm::quat parentRotation = parentTransform.GetWorldRotation();
      SetLocalRotation(glm::inverse(parentRotation) * rotation);
      return;
    }
  }

  SetLocalRotation(rotation);
}

// 缩放相关方法 ==============================================

void TransformComponent::SetLocalScale(const glm::vec3 &scale)
{
  if (m_Scale != scale) {
    m_Scale = scale;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    SetDirty();
  }
}

glm::vec3 TransformComponent::GetWorldScale() const
{
  const glm::mat4 &worldMat = GetWorldMatrix();
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(worldMat, scale, rotation, translation, skew, perspective);
  return scale;
}

// 矩阵相关方法 ==============================================

glm::mat4 TransformComponent::GetLocalMatrix() const
{
  if (m_LocalMatrixDirty) {
    m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::mat4_cast(m_Rotation) *
                    glm::scale(glm::mat4(1.0f), m_Scale);
    m_LocalMatrixDirty = false;
  }
  return m_LocalMatrix;
}

glm::mat4 TransformComponent::GetWorldMatrix() const
{
  if (m_WorldMatrixDirty) {
    CalculateWorldMatrix();
  }
  return m_WorldMatrix;
}

void TransformComponent::SetLocalMatrix(const glm::mat4 &matrix)
{
  DecomposeMatrix(matrix);
}

void TransformComponent::SetWorldMatrix(const glm::mat4 &matrix)
{
  if (GetOwner().lock()->HasComponent<HierarchyComponent>()) {
    auto &hierarchy = GetOwner().lock()->GetComponent<HierarchyComponent>();
    if (hierarchy.GetParent() != entt::null) {
      // 转换为局部矩阵
      TransformComponent &parentTransform =
          hierarchy.GetParent()->GetComponent<TransformComponent>();
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix();
      glm::mat4 localMat = glm::inverse(parentWorldMat) * matrix;
      DecomposeMatrix(localMat);
      return;
    }
  }
  DecomposeMatrix(matrix);
}

// 方向向量 ==============================================

glm::vec3 TransformComponent::Forward() const
{
  return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 TransformComponent::Up() const
{
  return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 TransformComponent::Right() const
{
  return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

// 组件接口实现 ==========================================

std::vector<std::type_index> TransformComponent::GetDependencies() const
{
  return {typeid(HierarchyComponent)};
}

bool TransformComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);  // 序列化基类数据

  // 序列化位置
  output.write(reinterpret_cast<const char *>(&m_Position), sizeof(m_Position));

  // 序列化旋转
  output.write(reinterpret_cast<const char *>(&m_Rotation), sizeof(m_Rotation));

  // 序列化缩放
  output.write(reinterpret_cast<const char *>(&m_Scale), sizeof(m_Scale));

  return !output.fail();
}

bool TransformComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  // 反序列化位置
  input.read(reinterpret_cast<char *>(&m_Position), sizeof(m_Position));

  // 反序列化旋转
  input.read(reinterpret_cast<char *>(&m_Rotation), sizeof(m_Rotation));

  // 反序列化缩放
  input.read(reinterpret_cast<char *>(&m_Scale), sizeof(m_Scale));

  // 标记矩阵需要更新
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;

  return !input.fail();
}

void TransformComponent::UpdateTransform()
{
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  SetDirty();
}

// 私有方法 ==============================================

void TransformComponent::CalculateWorldMatrix() const
{
  const glm::mat4 localMat = GetLocalMatrix();

  if (GetOwner().lock()->HasComponent<HierarchyComponent>()) {
    auto &hierarchy = GetOwner().lock()->GetComponent<HierarchyComponent>();
    if (hierarchy.GetParent() != entt::null) {
      // 如果有父节点，计算世界矩阵
      TransformComponent &parentTransform =
          hierarchy.GetParent()->GetComponent<TransformComponent>();
      m_WorldMatrix = parentTransform.GetWorldMatrix() * localMat;
    }
    else {
      // 没有父节点，局部矩阵就是世界矩阵
      m_WorldMatrix = localMat;
    }
  }
  else {
    // 没有层次组件，局部矩阵就是世界矩阵
    m_WorldMatrix = localMat;
  }

  m_WorldMatrixDirty = false;
}

void TransformComponent::DecomposeMatrix(const glm::mat4 &matrix)
{
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, m_Scale, m_Rotation, m_Position, skew, perspective);

  // 正交化旋转
  m_Rotation = glm::normalize(m_Rotation);

  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  SetDirty();
}
};
