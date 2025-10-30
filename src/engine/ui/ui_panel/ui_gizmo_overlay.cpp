#include "ui_gizmo_overlay.h"
#include "basic_event/render_event.h"
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

  // 相机旋转中心计算（通过cameradir和length确定的旋转中心）
  glm::vec3 cameraPos = context.cameraTransform.GetPosition();  // 相机位置
  glm::vec3 worldUp = Transform::GetWorldUp();                  // 世界Up方向
  glm::vec3 cameraDir = context.cameraTransform.GetForward();   // 相机看向方向
  glm::vec3 objectPos = context.modelTransform.GetPosition();   // 选中物体位置

  // 计算绕物体旋转的最优length，用于选中物体状态下的旋转
  float optimalLength = glm::length(objectPos - cameraPos) /
                        glm::abs(glm::dot(cameraDir, glm::normalize(objectPos - cameraPos)));

  // 非选中状态下的length使用固定值。确保不会转错
  float defaultLength = 10.0f;

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
                             optimalLength,
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

    // 绘制ViewManipulate，操控相机绕着距离为defaultLength的正前方的点旋转
    ImGuizmo::ViewManipulate(glm::value_ptr(viewMatrix),
                             glm::value_ptr(context.cameraProjection),
                             (ImGuizmo::OPERATION)m_CurrentOperation,
                             (ImGuizmo::MODE)m_CurrentMode,
                             glm::value_ptr(tempModelMatrix),
                             defaultLength,
                             viewManipulatePosition,
                             {m_ViewManipulateSize.x, m_ViewManipulateSize.y},
                             0x10101010);
  }
  // 临时变量反馈回Context
  context.cameraTransform.SetLocalMatrix(glm::inverse(viewMatrix));
  context.modelTransform.SetLocalMatrix(modelMatrix);

  // 发布选中物体变换事件（注意，这里是没有Dirty检测的，也就是每帧都会执行更新操作，向SceneGraph塞入Dirty节点）
  if (context.isModelSelected) {
    EventBus::Publish<ViewportPickedUpdateEvent>(
        ViewportPickedUpdateEvent(context.modelTransform));
  }

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