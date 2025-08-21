#include "camera.h"

namespace mite {
Camera::Camera()
{
  SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
  RecalculateProjection();
}

// === 投影参数设置 ===

void Camera::SetPerspective(float fov, float aspect, float near, float far)
{
  m_ProjectionType = ProjectionType::Perspective;
  m_FOV = fov;
  m_Aspect = aspect;
  m_Near = near;
  m_Far = far;
  RecalculateProjection();
}

void Camera::SetOrthographic(float size, float aspect, float near, float far)
{
  m_ProjectionType = ProjectionType::Orthographic;
  m_OrthoSize = size;
  m_Aspect = aspect;
  m_Near = near;
  m_Far = far;
  RecalculateProjection();
}

void Camera::SetProjectionType(ProjectionType type)
{
  m_ProjectionType = type;
  RecalculateProjection();
}

void Camera::SetAspectRatio(float aspect)
{
  m_Aspect = aspect;
  RecalculateProjection();
}

void Camera::LookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
  m_Position = position;

  // 计算看向目标的方向向量
  glm::vec3 direction = glm::normalize(target - position);

  // 直接从方向向量计算欧拉角
  m_RotationEuler.y = glm::degrees(atan2(-direction.x, -direction.z));  // yaw
  m_RotationEuler.x = glm::degrees(asin(direction.y));               // pitch
  m_RotationEuler.z = 0.0f;

  // 然后用欧拉角构建视图矩阵
  RecalculateViewFromRotation();
}
void Camera::SetViewMatrix(const glm::mat4 &view)
{
  m_ViewMatrix = view;

  // 更新位置：视图矩阵的逆矩阵的平移分量就是相机位置
  glm::mat4 inverseView = glm::inverse(view);
  m_Position = glm::vec3(inverseView[3]);

  // 从视图矩阵提取旋转矩阵（视图矩阵的左上3x3是相机到世界的旋转）
  glm::mat3 rotationMat = glm::mat3(view);

  // 更新欧拉角
  m_RotationEuler.y = glm::degrees(atan2(rotationMat[0][2], rotationMat[2][2]));  // yaw
  m_RotationEuler.x = glm::degrees(asin(-rotationMat[1][2]));                     // pitch
  m_RotationEuler.z = glm::degrees(atan2(rotationMat[1][0], rotationMat[1][1]));  // roll
}

// === 矩阵获取 ===

const glm::mat4 &Camera::GetProjectionMatrix() const
{
  return m_ProjectionMatrix;
}
const glm::mat4 &Camera::GetViewMatrix() const
{
  return m_ViewMatrix;
}
glm::mat4 Camera::GetViewProjectionMatrix() const
{
  return m_ProjectionMatrix * m_ViewMatrix;
}

// === 参数访问 ===

Camera::ProjectionType Camera::GetProjectionType() const
{
  return m_ProjectionType;
}

float Camera::GetNear() const
{
  return m_Near;
}
float Camera::GetFar() const
{
  return m_Far;
}
float Camera::GetFOV() const
{
  return m_FOV;
}
float Camera::GetAspectRatio() const
{
  return m_Aspect;
}
glm::vec3 Camera::GetPosition() const
{
  return m_Position;
}

glm::vec3 Camera::GetRightVector() const
{
  // （复用RecalculateViewFromRotation代码段）
  // 从欧拉角计算旋转四元数
  glm::quat rotation = glm::quat(glm::radians(m_RotationEuler));
  // 计算方向向量
  glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
  glm::vec3 up = rotation * glm::vec3(0, 1, 0);

  // 防止相机翻滚，强制上向量与世界Y轴对齐
  glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
  up = glm::normalize(glm::cross(right, forward));

  return right;
}

glm::vec3 Camera::GetUpVector() const
{
  // （复用RecalculateViewFromRotation代码段）
  // 从欧拉角计算旋转四元数
  glm::quat rotation = glm::quat(glm::radians(m_RotationEuler));
  // 计算方向向量
  glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
  glm::vec3 up = rotation * glm::vec3(0, 1, 0);

  // 防止相机翻滚，强制上向量与世界Y轴对齐
  glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
  up = glm::normalize(glm::cross(right, forward));

  return up;
}

glm::vec3 Camera::GetForwardVector() const
{
  // （复用RecalculateViewFromRotation代码段）
  // 从欧拉角计算旋转四元数
  glm::quat rotation = glm::quat(glm::radians(m_RotationEuler));
  // 计算方向向量
  glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
  glm::vec3 up = rotation * glm::vec3(0, 1, 0);

  // 防止相机翻滚，强制上向量与世界Y轴对齐
  glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
  up = glm::normalize(glm::cross(right, forward));

  return forward;
}

 float Camera::GetDistance() const
{
  // 计算相机位置到世界原点的距离
  return glm::length(GetPosition());
}

// === 相机控制方法实现 ===

void Camera::Rotate(float yaw, float pitch)
{
  // 累积旋转角度
  m_RotationEuler.y += yaw;    // 偏航（绕Y轴）
  m_RotationEuler.x += pitch;  // 俯仰（绕X轴）

  // 限制俯仰角度避免翻转
  m_RotationEuler.x = glm::clamp(m_RotationEuler.x, -89.0f, 89.0f);

  // 从旋转重建视图矩阵
  RecalculateViewFromRotation();
}

void Camera::Pan(float right, float up)
{
  // 屏幕空间平移转换为世界空间移动
  const glm::vec3 worldRight = GetRightVector() * right;
  const glm::vec3 worldUp = GetUpVector() * up;
  Move(worldRight + worldUp);
}

void Camera::Zoom(float amount)
{
  if (m_ProjectionType == ProjectionType::Perspective) {
    // 透视模式：调整FOV
    m_FOV = glm::clamp(m_FOV - amount, 1.0f, 170.0f);
    RecalculateProjection();
  }
  else {
    // 正交模式：调整视口大小
    m_OrthoSize = glm::max(m_OrthoSize - amount * 0.1f, 0.1f);
    RecalculateProjection();
  }
}

void Camera::Move(const glm::vec3 &direction)
{
  // 直接更新位置状态
  m_Position += direction;

  // 使用当前旋转重新构建视图矩阵
  RecalculateViewFromRotation();
}

// === 辅助方法 ===

void Camera::RecalculateViewFromRotation()
{
  // 从欧拉角计算旋转四元数
  glm::quat rotation = glm::quat(glm::radians(m_RotationEuler));

  // 计算方向向量
  glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
  glm::vec3 up = rotation * glm::vec3(0, 1, 0);

  // 防止相机翻滚，强制上向量与世界Y轴对齐
  glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
  up = glm::normalize(glm::cross(right, forward));

  // 构建视图矩阵
  m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, up);
}

void Camera::RecalculateProjection()
{
  // 透视相机
  if (m_ProjectionType == ProjectionType::Perspective) {
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
  }
  // 正交相机
  else if (m_ProjectionType == ProjectionType::Orthographic) {
    float width = m_OrthoSize * m_Aspect;
    float height = m_OrthoSize;
    m_ProjectionMatrix = glm::ortho(-width / 2, width / 2, -height / 2, height / 2, m_Near, m_Far);
  }
  else {
    LOG_ERROR("Invalid camera projection type");
  }
}

};  // namespace mite