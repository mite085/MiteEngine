#include "transform.h"

namespace mite {

const glm::vec3 Transform::s_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Transform::s_WorldForward = glm::vec3(0.0f, 0.0f, -1.0f);

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

void Transform::PanCamera(float horizontal, float vertical, const glm::vec3 &worldUp)
{
  // 执行水平/竖直方向平移
  Translate(GetConstrainedRight(worldUp) * horizontal);
  Translate(GetConstrainedUp(worldUp) * vertical);
}

// ==================== 旋转相关方法实现 ====================
// 旋转顺序
const Transform::EulerOrder &Transform::GetRotationOrder() const
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

// 欧拉角
const glm::vec3 &Transform::GetRotationEuler() const
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

// 四元数
const glm::quat &Transform::GetRotationQuat() const
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

// 世界轴旋转
void Transform::RotateWorld(const glm::vec3 &axis, float angleDegrees)
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
void Transform::RotateWorldX(float angleDegrees)
{
  RotateWorld(glm::vec3(1.0f, 0.0f, 0.0f), angleDegrees);
}
void Transform::RotateWorldY(float angleDegrees)
{
  RotateWorld(glm::vec3(0.0f, 1.0f, 0.0f), angleDegrees);
}
void Transform::RotateWorldZ(float angleDegrees)
{
  RotateWorld(glm::vec3(0.0f, 0.0f, 1.0f), angleDegrees);
}

// 局部轴旋转
void Transform::RotateLocal(const glm::vec3 &localAxis, float angleDegrees)
{
  // 将局部轴转换到世界空间
  glm::vec3 worldAxis = m_Rotation * localAxis;
  RotateWorld(worldAxis, angleDegrees);
}
void Transform::RotateLocalX(float angleDegrees)
{
  RotateLocal(glm::vec3(1.0f, 0.0f, 0.0f), angleDegrees);
}
void Transform::RotateLocalY(float angleDegrees)
{
  RotateLocal(glm::vec3(0.0f, 1.0f, 0.0f), angleDegrees);
}
void Transform::RotateLocalZ(float angleDegrees)
{
  RotateLocal(glm::vec3(0.0f, 0.0f, 1.0f), angleDegrees);
}

// 绕点旋转
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

// 偏航 - 俯仰 - 滚转旋转逻辑
void Transform::RotateYaw(float degrees, const glm::vec3 &worldUp)
{
  // 直接绕世界Up轴旋转
  RotateWorld(worldUp, degrees);
}
void Transform::RotatePitch(float degrees, const glm::vec3 &worldUp)
{
  // 获取当前的右向量（考虑防翻滚）
  glm::vec3 right = GetConstrainedRight(worldUp);

  // 预测旋转后的前向向量
  float radians = glm::radians(degrees);
  glm::quat predictedRotation = glm::angleAxis(radians, right) * m_Rotation;
  glm::vec3 predictedForward = predictedRotation * glm::vec3(0, 0, -1);   // 右手系相机：Forward为-Z方向

  // 计算预测的俯仰角
  float predictedPitch = glm::degrees(glm::asin(predictedForward.y));

  // 检查是否超出限制
  if (glm::abs(predictedPitch) <= 89.0f) {
    // 安全旋转
    RotateWorld(right, degrees);
  }
  else {
    // 钳制到边界
    float targetPitch = (predictedPitch > 0) ? 89.0f : -89.0f;
    float currentPitch = glm::degrees(glm::asin(GetForward().y));
    float clampedDegrees = targetPitch - currentPitch;

    if (glm::abs(clampedDegrees) > 0.1f) {
      RotateWorld(right, clampedDegrees);
    }
  }
}
void Transform::RotateRoll(float degrees, const glm::vec3 &worldUp)
{
  // 绕前向轴旋转（通常编辑器相机不需要）
  glm::vec3 forward = GetConstrainedForward(worldUp);
  RotateWorld(forward, degrees);
}
void Transform::RotateCamera(float yaw, float pitch, float roll, const glm::vec3 &worldUp)
{
  if (yaw != 0.0f)
    RotateYaw(yaw, worldUp);
  if (pitch != 0.0f)
    RotatePitch(pitch, worldUp);
  if (roll != 0.0f)
    RotateRoll(roll, worldUp);
}

// LookAt功能
void Transform::LookAt(const glm::vec3 &target, const glm::vec3 &up)
{
  // 使用glm的lookAt函数计算旋转
  glm::mat4 viewMatrix = glm::lookAt(m_Position, target, up);

  // 视图矩阵的逆为相机的旋转矩阵
  glm::mat3 rotationMat = glm::mat3(glm::inverse(viewMatrix));

  // 从旋转矩阵提取旋转四元数
  m_Rotation = glm::quat_cast(rotationMat);
  m_RotationDirty = true;
  m_MatrixDirty = true;
}

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

const glm::mat4 &Transform::GetLocalMatrix() const
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

const glm::mat4 Transform::GetViewMatrix() const
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

const glm::vec3 Transform::GetForward() const
{
  return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);  // 右手系相机：Forward为-Z方向
}

const glm::vec3 Transform::GetUp() const
{
  return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);  // 右手系相机：Up为+Y方向
}

const glm::vec3 Transform::GetRight() const
{
  return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);  // 右手系相机：Right为+X方向
}

const glm::vec3 Transform::GetConstrainedUp(const glm::vec3 &worldUp) const
{
  return worldUp;  // 强制使用指定的世界向上方向
}

const glm::vec3 Transform::GetConstrainedRight(const glm::vec3 &worldUp) const
{
  // 获取世界前向向量
  // （无论是否防翻滚，这个值应当是固定朝向Target的，应当以该值作为基准进行计算）
  glm::vec3 forward = GetForward(); 

  // 确保前向向量不与世界Up轴平行
  // （RotatePitch时会进行俯仰角度限制，基本不会触发该分支）
  if (glm::length(glm::cross(forward, worldUp)) < 0.001f) {
    // 如果平行，使用默认右向量
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }

  // 新的世界右向量 = 世界前向向量 × 世界Up向量
  return glm::normalize(glm::cross(forward, worldUp));
}

const glm::vec3 Transform::GetConstrainedForward(const glm::vec3 &worldUp) const
{
  glm::vec3 right = GetConstrainedRight(worldUp);

  // 世界前向向量 = 世界Up向量 × 右向量
  // （直接返回GetForward()应当也是一样的结果，前向向量不应当随着Up改变）
  return glm::normalize(glm::cross(worldUp, right));
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

void Transform::CleanDirty()
{
  if (m_MatrixDirty) {
    UpdateLocalMatrix();
  }
  if (m_RotationDirty) {
    UpdateEulerFromRotation();
  }
}

// ==================== 运算符重载实现 ====================
Transform operator*(const Transform &lhs, const Transform &rhs)
{
  // 继承lhs的变换模式
  return Transform(lhs.GetLocalMatrix() * rhs.GetLocalMatrix(), lhs.GetRotationOrder());
}
Transform operator*(const Transform &lhs, const glm::mat4 &rhs)
{
  // 将变换应用于矩阵：lhs.GetLocalMatrix() * rhs
  return Transform(lhs.GetLocalMatrix() * rhs, lhs.GetRotationOrder());
}
Transform operator*(const glm::mat4 &lhs, const Transform &rhs)
{
  // 将矩阵应用于变换：lhs * rhs.GetLocalMatrix()
  return Transform(lhs * rhs.GetLocalMatrix(), rhs.GetRotationOrder());
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

void Transform::UpdateRotationFromEuler() const
{
  m_Rotation = EulerToQuatByOrder(m_RotationEuler, m_RotationOrder);
  m_RotationDirty = false;
}
void Transform::UpdateEulerFromRotation() const
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