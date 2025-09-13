#include "camera.h"

namespace mite {
Camera::Camera()
{
  SetPerspective(45.0f, 16.0f / 9.0f, 0.01f, 1000.0f);
}

// === 投影参数设置 ===

void Camera::SetPerspective(float fov, float aspect, float near, float far)
{
  m_ProjectionType = ProjectionType::Perspective;
  m_FOV = fov;
  m_Aspect = aspect;
  m_Near = near;
  m_Far = far;
  m_ProjectionDirty = true;
}

void Camera::SetOrthographic(float size, float aspect, float near, float far)
{
  m_ProjectionType = ProjectionType::Orthographic;
  m_OrthoSize = size;
  m_Aspect = aspect;
  m_Near = near;
  m_Far = far;
  m_ProjectionDirty = true;
}

void Camera::SetProjectionType(ProjectionType type)
{
  m_ProjectionType = type;
  m_ProjectionDirty = true;
}

void Camera::SetAspectRatio(float aspect)
{
  m_Aspect = aspect;
  m_ProjectionDirty = true;
}

// === 矩阵获取 ===

const glm::mat4 &Camera::GetProjectionMatrix() const
{
  if (m_ProjectionDirty) {
    UpdateProjection();
  }
  return m_ProjectionMatrix;
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
float Camera::GetOrthoSize() const
{
  return m_OrthoSize;
}

// === 状态检查 ===
bool Camera::IsProjectionDirty() const
{
  return m_ProjectionDirty;
}
void Camera::MarkProjectionClean()
{
  m_ProjectionDirty = false;
}

// === 相机控制方法实现 ===
void Camera::Zoom(float amount)
{
  if (m_ProjectionType == ProjectionType::Perspective) {
    // 透视模式：调整FOV
    m_FOV = glm::clamp(m_FOV - amount, 1.0f, 170.0f);
    m_ProjectionDirty = true;
  }
  else {
    // 正交模式：调整视口大小
    m_OrthoSize = glm::max(m_OrthoSize - amount * 0.1f, 0.1f);
    m_ProjectionDirty = true;
  }
}

// === 辅助方法 ===

void Camera::UpdateProjection() const
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
    m_ProjectionMatrix = glm::mat4(1.0f);
  }

  m_ProjectionDirty = false;
}
};  // namespace mite