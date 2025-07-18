#include "camera.h"

namespace mite {
Camera::Camera()
{
  RecalculateProjection();
}

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
    LOG_ERROR("Invalid camera projection type: {}", m_ProjectionType);
  }
}

void Camera::LookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
  m_ViewMatrix = glm::lookAt(position, target, up);
}
void Camera::SetViewMatrix(const glm::mat4 &view)
{
  m_ViewMatrix = view;
}
};  // namespace mite