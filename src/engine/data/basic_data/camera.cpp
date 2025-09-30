#include "camera.h"

namespace mite {
Camera::Camera()
{
  SetPerspective(45.0f, 0.1f, 100.0f);
  SetAspectRatio(16.0f / 9.0f);
}

// === 投影参数设置 ===

void Camera::SetPerspective(float fov, float near, float far)
{
  m_ProjectionType = CameraProjectionType::PERSPECTIVE;
  m_FOV = fov;
  m_Near = near;
  m_Far = far;
  m_ProjectionDirty = true;
}

void Camera::SetOrthographic(float size, float near, float far)
{
  m_ProjectionType = CameraProjectionType::ORTHOGRAPHIC;
  m_OrthoSize = size;
  m_Near = near;
  m_Far = far;
  m_ProjectionDirty = true;
}

void Camera::SetProjectionType(CameraProjectionType type)
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

CameraProjectionType Camera::GetProjectionType() const
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
  if (m_ProjectionType == CameraProjectionType::PERSPECTIVE) {
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
  if (m_ProjectionType == CameraProjectionType::PERSPECTIVE) {

    // glm::perspective第一个参数T fovy 是垂直方向的视野角度，固定了垂直方向的视野范围
    // 使用glm::radians(m_FOV)，当m_Aspect改变时，水平视野随窗口自适应，垂直视野固定为视场角

    // 使用短边FOV模式：
    // 宽和高更短的一边决定fov，另一边自适应拉长，
    // 以确保视野范围足够宽广，仿照Blender的显示模式
    if (m_Aspect >= 1.0f) {
      // 宽高比 >= 1，使用垂直FOV（短边是高度）
      m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
    }
    else {
      // 宽高比 < 1，使用水平FOV（短边是宽度）
      // 将垂直FOV转换为水平FOV：fov_horizontal = 2 * atan(tan(fov_vertical/2) * aspect)
      float horizontalFOV = 2.0f * glm::atan(glm::tan(glm::radians(m_FOV) * 0.5f) / m_Aspect);
      m_ProjectionMatrix = glm::perspective(horizontalFOV, m_Aspect, m_Near, m_Far);
    }
  }
  // 正交相机
  else if (m_ProjectionType == CameraProjectionType::ORTHOGRAPHIC) {
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