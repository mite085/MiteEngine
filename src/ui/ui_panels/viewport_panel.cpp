#include "viewport_panel.h"
#include "input.h"
#include "renderer.h"

namespace mite {
// 初始化视口面板
ViewportPanel::ViewportPanel(Renderer &renderer) : m_renderer(renderer), UIPanel("Viewport")
{
  // 视口特有样式设置
  m_windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
  m_defaultSize = ImVec2(800, 600);
}

// 面板首次加载时初始化
void ViewportPanel::OnAttach()
{
  // 初始化投影矩阵（透视投影）
  m_projMatrix = glm::perspective(
      glm::radians(45.0f), m_defaultSize.x / m_defaultSize.y, 0.1f, 100.0f);
}

// 每帧更新相机逻辑
void ViewportPanel::OnUpdate(float dt)
{
  if (ImGui::IsWindowHovered()) {
    UpdateCamera(dt);
  }
}

// 主绘制函数
void ViewportPanel::DrawContent()
{
  // 1. 获取当前渲染视口尺寸
  auto viewportSize = ImGui::GetContentRegionAvail();

  // 2. 通知Renderer更新视口尺寸
  m_renderer.SetViewport(viewportSize.x, viewportSize.y);

  // 3. 获取场景FBO并绘制到ImGui
  intptr_t fbo = m_renderer.GetViewportFramebuffer();
  ImGui::Image(fbo, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

  // 4. 只有视口聚焦时才处理Gizmo
  if (ImGui::IsWindowFocused()) {
    // 绘制Gizmo操作工具栏
    DrawGizmoToolbar();

    // 如果有选中实体则绘制Gizmo
    if (m_selectedEntity != entt::null) {
      HandleGizmo();
    }
  }
}

// 绘制Gizmo操作模式选择工具栏
void ViewportPanel::DrawGizmoToolbar()
{
  // 使用ImGui的SameLine和分组实现水平布局
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

  // 第一组：变换模式
  ImGui::BeginGroup();
  if (ImGui::RadioButton("Translate", m_gizmoOp == ImGuizmo::TRANSLATE))
    m_gizmoOp = ImGuizmo::TRANSLATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", m_gizmoOp == ImGuizmo::ROTATE))
    m_gizmoOp = ImGuizmo::ROTATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", m_gizmoOp == ImGuizmo::SCALE))
    m_gizmoOp = ImGuizmo::SCALE;
  ImGui::EndGroup();

  // 第二组：坐标系模式（与第一组保持间距）
  ImGui::SameLine(0, 20);
  ImGui::BeginGroup();
  if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD))
    m_gizmoMode = ImGuizmo::WORLD;
  ImGui::SameLine();
  if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL))
    m_gizmoMode = ImGuizmo::LOCAL;
  ImGui::EndGroup();

  // 添加分隔线增强可读性
  ImGui::Separator();
  ImGui::PopStyleVar();
}

// 处理Gizmo变换操作
void ViewportPanel::HandleGizmo()
{
//  auto &registry = SceneCore::GetRegistry();
//  if (!registry.valid(m_selectedEntity))
//    return;
//
//  // 1. 获取实体的Transform组件
//  auto &transform = registry.get<Transform>(m_selectedEntity);
//
//  // 2. 准备ImGuizmo矩阵
//  glm::mat4 modelMatrix = transform.GetWorldMatrix();  // 假设Transform类有该方法
//
//  // 3. 设置ImGuizmo上下文
//  ImGuizmo::SetOrthographic(false);
//  ImGuizmo::SetDrawlist();
//  ImGuizmo::SetRect(ImGui::GetWindowPos().x,
//                    ImGui::GetWindowPos().y,
//                    ImGui::GetWindowWidth(),
//                    ImGui::GetWindowHeight());
//
//  // 4. 计算视图矩阵（需在相机移动时更新）
//  CalculateViewMatrix();
//
//  // 5. 执行Gizmo操作
//  ImGuizmo::Manipulate(glm::value_ptr(m_viewMatrix),
//                       glm::value_ptr(m_projMatrix),
//                       m_gizmoOp,
//                       m_gizmoMode,
//                       glm::value_ptr(modelMatrix));
//
//  // 6. 如果Gizmo有变化，则更新Transform
//  if (ImGuizmo::IsUsing()) {
//    transform.SetFromWorldMatrix(modelMatrix);  // 假设Transform有矩阵分解方法
//  }
}

// 相机控制逻辑
void ViewportPanel::UpdateCamera(float dt)
{
  //// 键盘移动（WASD控制）
  //if (Input::IsKeyPressed(GLFW_KEY_W))
  //  m_cameraPos += m_cameraSpeed * dt * m_cameraFront;
  //if (Input::IsKeyPressed(GLFW_KEY_S))
  //  m_cameraPos -= m_cameraSpeed * dt * m_cameraFront;

  //// 鼠标旋转控制（省略实现细节）
  //// ...

  //// 标记视图矩阵需要更新
  //m_viewMatrixDirty = true;
}

// 计算视图矩阵
void ViewportPanel::CalculateViewMatrix()
{
  m_viewMatrix = glm::lookAt(
      m_cameraPos, m_cameraPos + m_cameraFront, glm::vec3(0, 1, 0)  // 上向量
  );
}


};
