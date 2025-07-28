#include "viewport_panel.h"
#include "imgui.h"
#include "ImGuizmo.h"

namespace mite {
ViewportPanel::ViewportPanel() : UIPanel("Viewport")
{
  // 配置视口特有样式
  m_windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;
  m_defaultSize = ImVec2(1280, 720);
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

void ViewportPanel::OnAttach()
{
  // 初始化视口FBO
  m_viewport.fbo = Renderer::CreateFramebuffer(1280, 720);
}

void ViewportPanel::OnUpdate(float dt)
{
  // 只在视口激活时处理更新
  if (!m_isViewportHovered || !ImGui::IsWindowFocused())
    return;

  //=== 相机移动控制 ===//
  auto &camera = SceneCamera::GetMainCamera();
  const float moveSpeed = 5.0f * dt;  // 基础移动速度

  // 键盘WASD移动
  glm::vec3 movement{0.0f};
  if (ImGui::IsKeyDown(ImGuiKey_W))
    movement += camera.GetForward();
  if (ImGui::IsKeyDown(ImGuiKey_S))
    movement -= camera.GetForward();
  if (ImGui::IsKeyDown(ImGuiKey_A))
    movement -= camera.GetRight();
  if (ImGui::IsKeyDown(ImGuiKey_D))
    movement += camera.GetRight();

  // 按住Shift加速
  const bool isFastMode = ImGui::IsKeyDown(ImGuiKey_LeftShift);
  camera.Translate(movement * moveSpeed * (isFastMode ? 3.0f : 1.0f));

  //=== 视口尺寸变化检测 ===//
  if (m_viewport.width == 0 || m_viewport.height == 0)
    return;
  camera.SetAspectRatio(static_cast<float>(m_viewport.width) / m_viewport.height);
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

void ViewportPanel::DrawGizmo(Entity selected)
{
  auto &registry = SceneCore::GetRegistry();
  if (!registry.valid(selected))
    return;

  auto &transform = registry.get<TransformComponent>(selected);

  // 获取ImGuizmo所需矩阵
  glm::mat4 modelMatrix = transform.GetWorldMatrix();
  const auto &view = SceneCamera::GetMainCamera().GetViewMatrix();
  const auto &projection = SceneCamera::GetMainCamera().GetProjectionMatrix();

  // 设置ImGuizmo上下文
  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(
      ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_viewport.width, m_viewport.height);

  // Gizmo操作类型切换
  static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_1))
    currentOp = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_2))
    currentOp = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(ImGuiKey_3))
    currentOp = ImGuizmo::SCALE;

  // 执行Gizmo变换
  ImGuizmo::Manipulate(glm::value_ptr(view),
                       glm::value_ptr(projection),
                       currentOp,
                       ImGuizmo::LOCAL,
                       glm::value_ptr(modelMatrix));

  // 如果发生变换，更新ECS组件
  if (ImGuizmo::IsUsing()) {
    transform.FromWorldMatrix(modelMatrix);

    // 触发组件更新事件
    EventBus::Publish(
        ComponentChangedEvent{.entity = selected, .componentType = typeid(TransformComponent)});
  }
}

void ViewportPanel::ProcessViewportResize()
{
  // 获取当前ImGui窗口内容区域尺寸
  const ImVec2 currentSize = ImGui::GetContentRegionAvail();
  const auto newWidth = static_cast<uint32_t>(currentSize.x);
  const auto newHeight = static_cast<uint32_t>(currentSize.y);

  // 尺寸无变化时跳过
  if (newWidth == m_viewport.width && newHeight == m_viewport.height)
    return;

  // 验证最小尺寸
  const uint32_t minSize = 16;
  if (newWidth < minSize || newHeight < minSize)
    return;

  // 更新FBO尺寸
  Renderer::ResizeFramebuffer(m_viewport.fbo, newWidth, newHeight);
  m_viewport.width = newWidth;
  m_viewport.height = newHeight;

  // 通知渲染器视口尺寸变化
  EventBus::Publish(ViewportResizeEvent{.width = newWidth, .height = newHeight});

  // 更新相机纵横比
  auto &camera = SceneCamera::GetMainCamera();
  camera.SetAspectRatio(static_cast<float>(newWidth) / newHeight);
}

};
