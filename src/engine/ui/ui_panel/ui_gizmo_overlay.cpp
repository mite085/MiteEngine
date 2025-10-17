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
  ImVec2 viewManipulateSize = {128, 128};
  ImVec2 viewManipulatePosition = ImVec2(viewManipulateRight - viewManipulateSize.x,
                                         viewManipulateTop);

  // 相机与选中物体距离计算
  glm::vec3 cameraPos = glm::vec3(glm::inverse(context.viewMatrix)[3]);  // 相机位置
  glm::vec3 objectPos = glm::vec3(context.modelMatrix[3]);               // 选中物体位置
  float distanceToOrigin = glm::length(cameraPos);               // 计算到原点的距离
  float distanceToObject = glm::distance(cameraPos, objectPos);  // 计算到特定物体的距离

  // 设置ImGuizmo工作区域为整个ViewPort
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(context.viewportPos.x,
                    context.viewportPos.y,
                    context.viewportSize.x,
                    context.viewportSize.y);

  // 若为选中状态，绘制Manipulate以及ViewManipulate
  if (context.isModelSelected) {
    // 绘制ViewManipulate，操控相机绕着模型旋转
    ImGuizmo::ViewManipulate(glm::value_ptr(context.viewMatrix),
                             distanceToObject,
                             viewManipulatePosition,
                             viewManipulateSize,
                             0x10101010);
    // 渲染Gizmo
    ImGuizmo::Manipulate(glm::value_ptr(context.viewMatrix),
                         glm::value_ptr(context.projectionMatrix),
                         (ImGuizmo::OPERATION)m_CurrentOperation,
                         (ImGuizmo::MODE)m_CurrentMode,
                         glm::value_ptr(context.modelMatrix),
                         nullptr,
                         m_UseSnap ? glm::value_ptr(m_SnapValue) : nullptr);

    // 更新交互状态（暂未启用状态访问）
    m_IsUsing = ImGuizmo::IsUsing();
    m_IsOver = ImGuizmo::IsOver();
  }
  // 若非选中状态，仅绘制ViewManipulate
  else {
    glm::mat4 tempModelMatrix(1.0f); // 虽然ViewManipulate的float* matrix不参与计算，但不能为空

    // 绘制ViewManipulate，操控相机绕着原点旋转
    ImGuizmo::ViewManipulate(glm::value_ptr(context.viewMatrix),
                             glm::value_ptr(context.projectionMatrix),
                             (ImGuizmo::OPERATION)m_CurrentOperation,
                             (ImGuizmo::MODE)m_CurrentMode,
                             glm::value_ptr(tempModelMatrix),
                             distanceToOrigin,
                             viewManipulatePosition,
                             viewManipulateSize,
                             0x10101010);
  }
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