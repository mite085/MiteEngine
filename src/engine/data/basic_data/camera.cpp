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
  m_ViewMatrix = glm::lookAt(position, target, up);
}
void Camera::SetViewMatrix(const glm::mat4 &view)
{
  m_ViewMatrix = view;
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
  return -glm::vec3(m_ViewMatrix[3]) * glm::mat3(m_ViewMatrix);
}

glm::vec3 Camera::GetRightVector() const
{
  return glm::normalize(glm::vec3(m_ViewMatrix[0]));
}

glm::vec3 Camera::GetUpVector() const
{
  return glm::normalize(glm::vec3(m_ViewMatrix[1]));
}

glm::vec3 Camera::GetForwardVector() const
{
  return -glm::normalize(glm::vec3(m_ViewMatrix[2]));
}

// === 相机控制方法实现 ===

void Camera::Rotate(float yaw, float pitch)
{
  // 获取当前朝向和上向量
  const glm::vec3 forward = GetForwardVector();
  const glm::vec3 up = GetUpVector();

  // 构造当前旋转四元数
  const glm::quat orientation = glm::quatLookAt(forward, up);

  // 创建偏航和俯仰旋转四元数
  const glm::quat yawRot = glm::angleAxis(glm::radians(-yaw), glm::vec3(0, 1, 0));
  const glm::quat pitchRot = glm::angleAxis(glm::radians(pitch), GetRightVector());

  // 组合旋转
  const glm::quat newOrientation = yawRot * orientation * pitchRot;

  // 计算新方向向量
  const glm::vec3 newForward = newOrientation * glm::vec3(0, 0, -1);  // 四元数旋转向量
  const glm::vec3 newUp = newOrientation * glm::vec3(0, 1, 0);

  // 更新视图矩阵
  const glm::vec3 position = GetPosition();
  LookAt(position, position + newForward, newUp);
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
    m_FOV = glm::clamp(m_FOV - amount, 1.0f, 120.0f);
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
  // 直接修改视图矩阵的平移分量
  glm::vec3 position = GetPosition();
  position += direction;
  LookAt(position, position + GetForwardVector(), GetUpVector());
}

// === 辅助方法 ===

void Camera::RecalculateProjection()
{
  if (m_ProjectionType == ProjectionType::Perspective) {
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
  }
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