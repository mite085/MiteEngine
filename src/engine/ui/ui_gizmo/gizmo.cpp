#include "gizmo.h"

namespace mite {

Gizmo::Gizmo()
{
  ImGuizmo::Enable(true);
}

void Gizmo::SetOperation(ImGuizmo::OPERATION operation)
{
  m_Operation = operation;
}

void Gizmo::SetMode(ImGuizmo::MODE mode)
{
  m_Mode = mode;
}

void Gizmo::Show()
{
  m_IsVisible = true;
}

void Gizmo::Hide()
{
  m_IsVisible = false;
}

void Gizmo::Toggle()
{
  m_IsVisible = !m_IsVisible;
}

bool Gizmo::IsVisible() const
{
  return m_IsVisible;
}

void Gizmo::SetVisible(bool visible)
{
  m_IsVisible = visible;
}

bool Gizmo::Manipulate(glm::mat4 &transformMatrix,
                       const Camera &camera,
                       const glm::vec2 &viewportPos,
                       const glm::vec2 &viewportSize)
{
  // 如果Gizmo不可见，直接返回false
  if (!m_IsVisible) {
    return false;
  }

  // 设置Gizmo操作空间
  ImGuizmo::SetOrthographic(camera.GetProjectionType() == Camera::ProjectionType::Orthographic);
  ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

  // 执行Gizmo操作
  bool manipulated = ImGuizmo::Manipulate(glm::value_ptr(camera.GetViewMatrix()),
                                          glm::value_ptr(camera.GetProjectionMatrix()),
                                          m_Operation,
                                          m_Mode,
                                          glm::value_ptr(transformMatrix));

  m_IsUsing = ImGuizmo::IsUsing();
  m_IsOver = ImGuizmo::IsOver();

  return manipulated;
}

bool Gizmo::IsUsing() const
{
  return m_IsUsing;
}

bool Gizmo::IsOver() const
{
  return m_IsOver;
}

ImGuizmo::OPERATION Gizmo::GetOperation() const
{
  return m_Operation;
}

ImGuizmo::MODE Gizmo::GetMode() const
{
  return m_Mode;
}

}  // namespace mite