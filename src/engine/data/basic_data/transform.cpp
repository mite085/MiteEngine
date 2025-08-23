#include "transform.h"

namespace mite {

Transform::Transform()
{
  // 默认构造已经是单位变换
}

Transform::Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
    : m_Position(position), m_RotationEuler(rotation), m_Scale(scale)
{
  // 从欧拉角初始化四元数
  UpdateRotationFromEuler();
  m_MatrixDirty = true;
}

Transform::Transform(const glm::mat4 &matrix)
{
  SetLocalMatrix(matrix);
}

// ==================== 位置相关方法实现 ====================

const glm::vec3 &Transform::GetPosition() const
{
  return m_Position;
}

void Transform::SetPosition(const glm::vec3 &position)
{
  if (m_Position != position) {
    m_Position = position;
    m_MatrixDirty = true;
  }
}

void Transform::Translate(const glm::vec3 &direction)
{
  m_Position += direction;
  m_MatrixDirty = true;
}

// ==================== 旋转相关方法实现 ====================

glm::vec3 Transform::GetRotation() const
{
  return m_RotationEuler;
}

void Transform::SetRotation(const glm::vec3 &rotation)
{
  if (m_RotationEuler != rotation) {
    m_RotationEuler = rotation;
    UpdateRotationFromEuler();
    m_MatrixDirty = true;
  }
}

void Transform::SetRotation(float x, float y, float z)
{
  SetRotation(glm::vec3(x, y, z));
}

void Transform::Rotate(const glm::vec3 &axis, float angle)
{
  // 转换为弧度
  float radians = glm::radians(angle);

  // 确保旋转轴是单位向量
  const glm::vec3 normalizedAxis = glm::normalize(axis);

  // 创建旋转四元数
  const glm::quat rotation = glm::angleAxis(radians, normalizedAxis);

  // 应用旋转（世界空间）
  m_Rotation = glm::normalize(rotation * m_Rotation);

  // 更新欧拉角
  UpdateEulerFromRotation();
  m_MatrixDirty = true;
}

void Transform::RotateX(float angle)
{
  Rotate(glm::vec3(1.0f, 0.0f, 0.0f), angle);
}

void Transform::RotateY(float angle)
{
  Rotate(glm::vec3(0.0f, 1.0f, 0.0f), angle);
}

void Transform::RotateZ(float angle)
{
  Rotate(glm::vec3(0.0f, 0.0f, 1.0f), angle);
}

void Transform::RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angle)
{
  // 转换为弧度
  float radians = glm::radians(angle);

  // 创建旋转四元数
  const glm::vec3 normalizedAxis = glm::normalize(axis);
  const glm::quat rotation = glm::angleAxis(radians, normalizedAxis);

  // 计算从旋转中心到当前位置的向量
  const glm::vec3 toObject = m_Position - point;

  // 旋转该向量
  const glm::vec3 rotatedVec = rotation * toObject;

  // 计算新位置
  m_Position = point + rotatedVec;

  // 应用旋转到当前旋转
  m_Rotation = glm::normalize(rotation * m_Rotation);

  // 更新欧拉角
  UpdateEulerFromRotation();
  m_MatrixDirty = true;
}

void Transform::LookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
  m_Position = position;

  // 计算看向目标的方向向量
  glm::vec3 direction = glm::normalize(target - position);

  // 直接从方向向量计算欧拉角（复用Camera的算法）
  m_RotationEuler.y = glm::degrees(atan2(-direction.x, -direction.z));  // yaw
  m_RotationEuler.x = glm::degrees(asin(direction.y));                  // pitch
  m_RotationEuler.z = 0.0f;

  // 限制俯仰角度避免翻转
  m_RotationEuler.x = glm::clamp(m_RotationEuler.x, -89.0f, 89.0f);

  // 更新四元数旋转
  UpdateRotationFromEuler();
  m_MatrixDirty = true;
}

// ==================== 缩放相关方法实现 ====================

const glm::vec3 &Transform::GetScale() const
{
  return m_Scale;
}

void Transform::SetScale(const glm::vec3 &scale)
{
  if (m_Scale != scale) {
    m_Scale = scale;
    m_MatrixDirty = true;
  }
}

void Transform::SetScale(float scale)
{
  SetScale(glm::vec3(scale, scale, scale));
}

// ==================== 矩阵相关方法实现 ====================

glm::mat4 Transform::GetLocalMatrix() const
{
  if (m_MatrixDirty) {
    UpdateLocalMatrix();
  }
  return m_LocalMatrix;
}

void Transform::SetLocalMatrix(const glm::mat4 &matrix)
{
  // 分解矩阵到TRS组件
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, m_Scale, m_Rotation, m_Position, skew, perspective);

  // 更新欧拉角
  UpdateEulerFromRotation();

  // 直接更新缓存矩阵
  m_LocalMatrix = matrix;
  m_MatrixDirty = false;
}

glm::mat4 Transform::GetWorldMatrix() const
{
  // 对于独立Transform，世界矩阵就是局部矩阵
  return GetLocalMatrix();
}

// ==================== 方向向量方法实现 ====================

glm::vec3 Transform::GetForward() const
{
  return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Transform::GetUp() const
{
  return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Transform::GetRight() const
{
  return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

// ==================== 辅助方法实现 ====================

void Transform::Reset()
{
  m_Position = glm::vec3(0.0f);
  m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  m_RotationEuler = glm::vec3(0.0f);
  m_Scale = glm::vec3(1.0f);
  m_MatrixDirty = true;
}

bool Transform::IsIdentity() const
{
  return m_Position == glm::vec3(0.0f) && m_Rotation == glm::quat(1.0f, 0.0f, 0.0f, 0.0f) &&
         m_Scale == glm::vec3(1.0f);
}

// ==================== 私有方法实现 ====================

void Transform::UpdateLocalMatrix() const
{
  m_LocalMatrix = glm::mat4(1.0f);
  m_LocalMatrix = glm::translate(m_LocalMatrix, m_Position);
  m_LocalMatrix *= glm::mat4_cast(m_Rotation);
  m_LocalMatrix = glm::scale(m_LocalMatrix, m_Scale);
  m_MatrixDirty = false;
}

void Transform::UpdateRotationFromEuler()
{
  // 将欧拉角转换为弧度
  glm::vec3 radians = glm::radians(m_RotationEuler);

  // 使用YXZ顺序（偏航-俯仰-滚转）创建四元数
  m_Rotation = glm::quat(radians);
}

void Transform::UpdateEulerFromRotation()
{
  // 从四元数提取欧拉角（弧度）
  glm::vec3 radians = glm::eulerAngles(m_Rotation);

  // 转换为度
  m_RotationEuler = glm::degrees(radians);
}

}  // namespace mite