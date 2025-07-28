#include "viewport_panel.h"
#include <imgui.h>

namespace mite {
ViewportPanel::ViewportPanel() : UIPanel("Viewport")
{
  // 配置视口特有样式
  m_windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;
  m_defaultSize = ImVec2(1280, 720);
}

void ViewportPanel::OnAttach()
{
  // 初始化视口FBO
  m_viewport.fbo = Renderer::CreateFramebuffer(1280, 720);
}

void ViewportPanel::DrawContent()
{
  // 1. 菜单栏
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("视图")) {
      ImGui::MenuItem("显示网格", nullptr, &m_showGrid);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // 2. 获取当前视口区域
  ImVec2 viewportSize = ImGui::GetContentRegionAvail();
  if (viewportSize.x != m_viewport.width || viewportSize.y != m_viewport.height) {
    m_viewport.width = viewportSize.x;
    m_viewport.height = viewportSize.y;
    Renderer::ResizeFramebuffer(m_viewport.fbo, viewportSize.x, viewportSize.y);
  }

  // 3. 绑定FBO并渲染场景
  Renderer::BeginViewport(m_viewport.fbo);
  {
    // 从SceneView获取渲染数据
    auto renderData = SceneView::GetRenderableData();
    Renderer::SubmitScene(renderData);

    // 调试网格
    if (m_showGrid) {
      Renderer::DrawGrid(100, 1.0f);
    }
  }
  Renderer::EndViewport();

  // 4. 显示渲染结果
  ImGui::Image(reinterpret_cast<void *>(m_viewport.fbo.textureID),
               viewportSize,
               ImVec2(0, 1),
               ImVec2(1, 0)  // OpenGL纹理坐标修正
  );

  // 5. 交互处理
  m_isViewportHovered = ImGui::IsItemHovered();
  if (m_isViewportHovered) {
    HandleCameraControl();
    if (auto selected = SelectionSystem::GetSelected()) {
      DrawGizmo(selected);
    }
  }
}

void ViewportPanel::HandleCameraControl()
{
  // 鼠标右键旋转相机
  if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
    auto delta = ImGui::GetMouseDragDelta();
    m_cameraYaw += delta.x * 0.3f;
    m_cameraPitch -= delta.y * 0.3f;
    ImGui::ResetMouseDragDelta();

    // 更新场景相机
    auto &camera = SceneCamera::GetMainCamera();
    camera.SetRotation(m_cameraYaw, m_cameraPitch);
  }

  // WASD移动控制
  if (ImGui::IsKeyDown(ImGuiKey_W)) /* 前移逻辑 */
    ;
  if (ImGui::IsKeyDown(ImGuiKey_S)) /* 后移逻辑 */
    ;
}
};
