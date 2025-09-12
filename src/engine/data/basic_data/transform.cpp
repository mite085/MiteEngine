#include "transform.h"

namespace mite {

Transform::Transform(EulerOrder order) : m_RotationOrder(order)
{
  // 默认构造已经是单位变换
}

Transform::Transform(const glm::vec3 &position,
                     const glm::vec3 &rotationEuler,
                     const glm::vec3 &scale,
                     EulerOrder order)
    : m_Position(position), m_RotationEuler(rotationEuler), m_Scale(scale), m_RotationOrder(order)
{
  UpdateRotationFromEuler();
  m_MatrixDirty = true;
}

Transform::Transform(const glm::mat4 &matrix, EulerOrder order) : m_RotationOrder(order)
{
  SetLocalMatrix(matrix);
}

// ==================== 位置相关方法实现 ====================

const glm::vec3 &Transform::GetPosition() const
{
  // 在SetPosition()时Mark，在GetPosition()时执行Update()并消除Mark
  if (m_MatrixDirty) {
    UpdateLocalMatrix();
  }
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
glm::vec3 Transform::GetRotationEuler()
{
  if (m_RotationDirty) {
    UpdateEulerFromRotation();
  }
  return m_RotationEuler;
}
void Transform::SetRotationEuler(const glm::vec3 &eulerDegrees)
{
  // 四元数负责旋转计算，所以需要优先保证四元数的正确
  // SetEuler()之后需要立即Update四元数
  if (m_RotationEuler != eulerDegrees) {
    m_RotationEuler = eulerDegrees;
    UpdateRotationFromEuler();
    m_MatrixDirty = true;
  }
}
void Transform::SetRotationEuler(float x, float y, float z)
{
  SetRotationEuler(glm::vec3(x, y, z));
}
glm::quat Transform::GetRotationQuat() const
{
  return m_Rotation;
}
void Transform::SetRotationQuat(const glm::quat &rotation)
{
  // 欧拉角负责对外接口，所以无需优先保证欧拉角的正确
  // Mark RotationDirty，等待GetEuler()时再Update欧拉角即可。
  if (m_Rotation != rotation) {
    m_Rotation = rotation;
    m_RotationDirty = true;
    m_MatrixDirty = true;
  }
}
Transform::EulerOrder Transform::GetRotationOrder() const
{
  return m_RotationOrder;
}
void Transform::SetRotationOrder(EulerOrder order)
{
  // 旋转顺序改变应当导致对外显示的欧拉角改变
  // 不应当导致内部四元数的修改
  if (m_RotationOrder != order) {
    m_RotationOrder = order;
    m_RotationDirty = true;
    m_MatrixDirty = true;
  }
}
void Transform::Rotate(const glm::vec3 &axis, float angleDegrees)
{
  // 转换为弧度
  float radians = glm::radians(angleDegrees);

  // 创建旋转四元数
  glm::vec3 normalizedAxis = glm::normalize(axis);
  glm::quat rotation = glm::angleAxis(radians, normalizedAxis);

  // 直接将旋转四元数作用于m_Rotation
  m_Rotation = glm::normalize(rotation * m_Rotation);
  m_RotationDirty = true;
  m_MatrixDirty = true;
}
void Transform::RotateX(float angleDegrees)
{
  Rotate(glm::vec3(1.0f, 0.0f, 0.0f), angleDegrees);
}
void Transform::RotateY(float angleDegrees)
{
  Rotate(glm::vec3(0.0f, 1.0f, 0.0f), angleDegrees);
}
void Transform::RotateZ(float angleDegrees)
{
  Rotate(glm::vec3(0.0f, 0.0f, 1.0f), angleDegrees);
}
void Transform::RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angleDegrees)
{
  // 转换为弧度
  float radians = glm::radians(angleDegrees);

  // 创建旋转四元数
  glm::vec3 normalizedAxis = glm::normalize(axis);
  glm::quat rotation = glm::angleAxis(radians, normalizedAxis);

  // 计算从旋转中心到当前位置的向量
  glm::vec3 toObject = m_Position - point;
  // 旋转该向量
  glm::vec3 rotatedVec = rotation * toObject;
  // 计算新的位置
  m_Position = point + rotatedVec;

  // 将旋转四元数作用于m_Rotation
  m_Rotation = glm::normalize(rotation * m_Rotation);
  m_RotationDirty = true;
  m_MatrixDirty = true;
}

void Transform::RotateWithUpConstraint(float yaw,
                                       float pitch,
                                       float roll,
                                       const glm::vec3 &worldUp)
{
  RotateWithUpConstraint(glm::vec3(pitch, yaw, roll), worldUp);
}
void Transform::RotateWithUpConstraint(const glm::vec3 &eulerDelta, const glm::vec3 &worldUp)
{
  // 应用偏航旋转（绕世界Y轴）
  if (eulerDelta.y != 0.0f) {
    Rotate(worldUp, eulerDelta.y);  // 绕世界向上轴旋转
  }

  // 应用俯仰旋转（绕本地X轴）
  if (eulerDelta.x != 0.0f) {
    glm::vec3 right = GetConstrainedRight(worldUp);
    Rotate(right, eulerDelta.x);

    // 限制俯仰角度避免翻转
    glm::vec3 forward = GetConstrainedForward(worldUp);
    float currentPitch = glm::degrees(asin(forward.z));  // 假设Z是向上方向

    // 如果超过限制，回滚旋转
    if (abs(currentPitch) > 89.0f) {
      Rotate(right, -eulerDelta.x);  // 撤销本次俯仰旋转
    }
  }

  // 通常编辑器相机不需要滚转，但保留接口
  if (eulerDelta.z != 0.0f) {
    glm::vec3 forward = GetConstrainedForward(worldUp);
    Rotate(forward, eulerDelta.z);
  }
}

void Transform::LookAt(const glm::vec3 &target, const glm::vec3 &up)
{
  // 使用glm的lookAt函数计算旋转
  glm::mat4 viewMatrix = glm::lookAt(m_Position, target, up);
  glm::mat3 rotationMat = glm::mat3(viewMatrix);

  // 从视图矩阵提取旋转四元数
  m_Rotation = glm::quat_cast(rotationMat);
  m_RotationDirty = true;
  m_MatrixDirty = true;
}

// 旧版本使用degrees手动计算yaw和pitch的逻辑
//void Transform::LookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
//{
//  m_Position = position;
//
//  // 计算看向目标的方向向量
//  glm::vec3 direction = glm::normalize(target - position);
//
//  // 直接从方向向量计算欧拉角（复用Camera的算法）
//  m_RotationEuler.y = glm::degrees(atan2(-direction.x, -direction.z));  // yaw
//  m_RotationEuler.x = glm::degrees(asin(direction.y));                  // pitch
//  m_RotationEuler.z = 0.0f;
//
//  // 限制俯仰角度避免翻转
//  m_RotationEuler.x = glm::clamp(m_RotationEuler.x, -89.0f, 89.0f);
//
//  // 更新四元数旋转
//  UpdateRotationFromEuler();
//  m_MatrixDirty = true;
//}

// ==================== 缩放相关方法实现 ====================
const glm::vec3 &Transform::GetScale() const
{
  // 在SetScale()时Mark，在GetScale()时执行Update()并消除Mark
  if (m_MatrixDirty) {
    UpdateLocalMatrix();
  }
  return m_Scale;
}
void Transform::SetScale(const glm::vec3 &scale)
{
  if (m_Scale != scale) {
    m_Scale = scale;
    m_MatrixDirty = true;
  }
}
void Transform::SetScale(float uniformScale)
{
  SetScale(glm::vec3(uniformScale));
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

  // 标记RotationDirty，GetEuler()时根据EulerOrder重新计算，此处不接触EulerOrder
  m_RotationDirty = true;
  m_MatrixDirty = false;
  m_LocalMatrix = matrix;
}

glm::mat4 Transform::GetViewMatrix() const
{
  // 正常的转换矩阵，是将模型顶点的Local坐标转换为World坐标
  // 而Camera的View矩阵则是将World坐标转换到Camera的Local坐标系内
  // 所以Camera的View矩阵就应当是LocalMatrix的逆矩阵
  return glm::inverse(GetLocalMatrix());
}
bool Transform::IsViewMatrixValid() const
{
  return glm::determinant(GetLocalMatrix()) != 0.0f;
}

// ==================== 方向向量方法实现 ====================

glm::vec3 Transform::GetForward() const
{
  return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);  // 右手系相机：Forward为-Z方向
}

glm::vec3 Transform::GetUp() const
{
  return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);  // 右手系相机：Up为+Y方向
}

glm::vec3 Transform::GetRight() const
{
  return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);  // 右手系相机：Right为+X方向
}

glm::vec3 Transform::GetForward(const glm::vec3 &up) const
{
  // 基于指定上方向计算前向向量
  glm::vec3 right = glm::normalize(glm::cross(GetForward(), up));
  return glm::normalize(glm::cross(up, right));
}
glm::vec3 Transform::GetRight(const glm::vec3 &up) const
{
  // 基于指定上方向计算右向量
  return glm::normalize(glm::cross(GetForward(), up));
}

glm::vec3 Transform::GetConstrainedForward(const glm::vec3 &worldUp) const
{
  glm::vec3 forward = GetForward();
  glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
  return glm::normalize(glm::cross(worldUp, right));
}
glm::vec3 Transform::GetConstrainedUp(const glm::vec3 &worldUp) const
{
  return worldUp;  // 强制使用指定的世界向上方向
}
glm::vec3 Transform::GetConstrainedRight(const glm::vec3 &worldUp) const
{
  glm::vec3 forward = GetConstrainedForward(worldUp);
  return glm::normalize(glm::cross(forward, worldUp));
}

// ==================== 辅助方法实现 ====================

void Transform::Reset()
{
  m_Position = glm::vec3(0.0f);
  m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  m_RotationEuler = glm::vec3(0.0f);
  m_Scale = glm::vec3(1.0f);
  m_RotationOrder = EulerOrder::YXZ;
  m_MatrixDirty = true;
  m_RotationDirty = true;
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
  m_Rotation = EulerToQuatByOrder(m_RotationEuler, m_RotationOrder);
  m_RotationDirty = false;
}
void Transform::UpdateEulerFromRotation() 
{
  m_RotationEuler = QuatToEulerByOrder(m_Rotation, m_RotationOrder);
  m_RotationDirty = false;
}

glm::vec3 Transform::QuatToEulerByOrder(const glm::quat &quat, EulerOrder order) const
{
  // 先将四元数转换为旋转矩阵
  glm::mat4 rotationMatrix = glm::mat4_cast(quat);

  glm::vec3 radians(0.0f);

  switch (order) {
    case EulerOrder::XYZ: {
      glm::extractEulerAngleXYZ(rotationMatrix, radians.x, radians.y, radians.z);
      break;
    }
    case EulerOrder::XZY: {
      glm::extractEulerAngleXYZ(rotationMatrix, radians.x, radians.z, radians.y);
      break;
    }
    case EulerOrder::YXZ: {
      glm::extractEulerAngleYXZ(rotationMatrix, radians.y, radians.x, radians.z);
      break;
    }
    case EulerOrder::YZX: {
      glm::extractEulerAngleYXZ(rotationMatrix, radians.y, radians.z, radians.x);
      break;
    }
    case EulerOrder::ZXY: {
      glm::extractEulerAngleZXY(rotationMatrix, radians.z, radians.x, radians.y);
      break;
    }
    case EulerOrder::ZYX: {
      glm::extractEulerAngleZYX(rotationMatrix, radians.z, radians.y, radians.x);
      break;
    }
    default: {
      // 默认使用XYZ顺序
      glm::extractEulerAngleXYZ(rotationMatrix, radians.x, radians.y, radians.z);
      break;
    }
  }

  return glm::degrees(radians);
}

glm::quat Transform::EulerToQuatByOrder(const glm::vec3 &eulerDegrees, EulerOrder order) const
{
  glm::vec3 radians = glm::radians(eulerDegrees);

  switch (order) {
    case EulerOrder::XYZ:
      return glm::quat_cast(glm::eulerAngleXYZ(radians.x, radians.y, radians.z));
    case EulerOrder::XZY:
      return glm::quat_cast(glm::eulerAngleXYZ(radians.x, radians.y, radians.z));
    case EulerOrder::YXZ:
      return glm::quat_cast(glm::eulerAngleYXZ(radians.y, radians.x, radians.z));
    case EulerOrder::YZX:
      return glm::quat_cast(glm::eulerAngleYZX(radians.y, radians.z, radians.x));
    case EulerOrder::ZXY:
      return glm::quat_cast(glm::eulerAngleZXY(radians.z, radians.x, radians.y));
    case EulerOrder::ZYX:
      return glm::quat_cast(glm::eulerAngleZYX(radians.z, radians.y, radians.x));
    default:
      return glm::quat_cast(glm::eulerAngleXYZ(radians.x, radians.y, radians.z));
  }
}
}  // namespace mite