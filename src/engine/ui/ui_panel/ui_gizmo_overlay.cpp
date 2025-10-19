#include "ui_gizmo_overlay.h"
#include "imgui.h"
#include "imguizmo.h"  // 必须在"imgui.h"后面

namespace mite {
GizmoOverlay::GizmoOverlay()
{
  // 默认启用平移/旋转/缩放复合操作，以及世界坐标系
  m_CurrentOperation = ImGuizmo::OPERATION::UNIVERSAL;
  m_CurrentMode = ImGuizmo::MODE::WORLD;
}

void GizmoOverlay::Update(float deltaTime) {}

void GizmoOverlay::Render(OverlayContext &context)
{
  if (!m_Enabled || !m_Visible)
    return;
  if (context.viewportPos.x <= 0 || context.viewportPos.y <= 0)
    return;

  // ViewManipulate绘制区域计算
  float viewManipulateRight = context.viewportPos.x + context.viewportSize.x;
  float viewManipulateTop = context.viewportPos.y;
  ImVec2 viewManipulatePosition = ImVec2(viewManipulateRight - m_ViewManipulateSize.x,
                                         viewManipulateTop);

  // 相机与选中物体距离计算
  glm::vec3 cameraPos = context.cameraTransform.GetPosition();   // 相机位置
  glm::vec3 objectPos = context.modelTransform.GetPosition();    // 选中物体位置
  float distanceToOrigin = glm::length(cameraPos);               // 计算到原点的距离
  float distanceToObject = glm::distance(cameraPos, objectPos);  // 计算到特定物体的距离

  // 设置ImGuizmo工作区域为整个ViewPort
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(context.viewportPos.x,
                    context.viewportPos.y,
                    context.viewportSize.x,
                    context.viewportSize.y);

  // 创建临时变量用于Imgui操控
  glm::mat4 viewMatrix = context.cameraTransform.GetViewMatrix();
  glm::mat4 modelMatrix = context.modelTransform.GetLocalMatrix();

  // 若为选中状态，绘制Manipulate以及ViewManipulate
  if (context.isModelSelected) {
    // 绘制ViewManipulate，操控相机绕着模型旋转
    ImGuizmo::ViewManipulate(glm::value_ptr(viewMatrix),
                             distanceToObject,
                             viewManipulatePosition,
                             {m_ViewManipulateSize.x, m_ViewManipulateSize.y},
                             0x10101010);
    // 渲染Gizmo
    ImGuizmo::Manipulate(glm::value_ptr(viewMatrix),
                         glm::value_ptr(context.cameraProjection),
                         (ImGuizmo::OPERATION)m_CurrentOperation,
                         (ImGuizmo::MODE)m_CurrentMode,
                         glm::value_ptr(modelMatrix),
                         nullptr,
                         m_UseSnap ? glm::value_ptr(m_SnapValue) : nullptr);
  }
  // 若非选中状态，仅绘制ViewManipulate
  else {
    glm::mat4 tempModelMatrix(1.0f);  // 虽然ViewManipulate的float* matrix不参与计算，但不能为空

    // 绘制ViewManipulate，操控相机绕着原点旋转
    ImGuizmo::ViewManipulate(glm::value_ptr(viewMatrix),
                             glm::value_ptr(context.cameraProjection),
                             (ImGuizmo::OPERATION)m_CurrentOperation,
                             (ImGuizmo::MODE)m_CurrentMode,
                             glm::value_ptr(tempModelMatrix),
                             distanceToOrigin,
                             viewManipulatePosition,
                             {m_ViewManipulateSize.x, m_ViewManipulateSize.y},
                             0x10101010);
  }

  // 临时变量反馈回Context
  context.cameraTransform.SetLocalMatrix(glm::inverse(viewMatrix));
  context.modelTransform.SetLocalMatrix(modelMatrix);

  // 若鼠标处于viewManipulate区域内，也认为是使用中。
  if (context.mousePos.x > viewManipulatePosition.x &&
      context.mousePos.y > viewManipulatePosition.y &&
      context.mousePos.x < viewManipulatePosition.x + m_ViewManipulateSize.x &&
      context.mousePos.y < viewManipulatePosition.y + m_ViewManipulateSize.y)
  {
    m_IsUsing = true;
  }
  else {
    // 否则交给Imguizmo判断
    m_IsUsing = ImGuizmo::IsUsing();
  }

  // Over逻辑完全由Imguizmo判断
  m_IsOver = ImGuizmo::IsOver();
}

void GizmoOverlay::SetOperation(int operation)
{
  m_CurrentOperation = operation;
}

void GizmoOverlay::SetMode(int mode)
{
  m_CurrentMode = mode;
}
}  // namespace mite