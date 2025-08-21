#include "viewport_panel.h"
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &title) : UIPanel(title){}

ViewportPanel::~ViewportPanel()
{
  // 确保输入上下文已注销
  if (m_inputContext) {
    Input::PopContext();
  }
}

void ViewportPanel::onAttach()
{
  // 创建模块化输入上下文
  m_inputContext = std::make_shared<ModularInputContext>("Viewport");
  m_inputContext->SetBlockInput(false);

  // 初始化视口导航处理器
  if (m_camera) {
    m_viewportInput = std::make_shared<ViewportInputProcessor>(m_camera, GLFW_MOUSE_BUTTON_RIGHT);
    m_inputContext->AddProcessor(m_viewportInput);
  }

  // 注册输入上下文
  Input::PushContext(m_inputContext);
}

void ViewportPanel::onDetach()
{
  // 移除输入上下文
  Input::PopContext();
  m_inputContext.reset();
}

void ViewportPanel::onUpdate(float deltaTime)
{
  // 更新视口输入状态
  if (m_viewportInput) {
    m_viewportInput->SetViewportHovered(m_viewportHovered);
    m_viewportInput->SetViewportFocused(m_viewportFocused);
  }

  // 更新相机导航(持续移动等)
  if (m_viewportInput && m_viewportFocused && m_viewportHovered) {
    m_viewportInput->UpdateCameraTransform(deltaTime);
  }
}

void ViewportPanel::onRender()
{
  // 设置视口窗口样式(无内边距)
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin(m_title.c_str(), &m_visible);

  // ===== 1. 更新视口状态 =====
  m_viewportFocused = ImGui::IsWindowFocused();
  m_viewportHovered = ImGui::IsWindowHovered();
  updateViewportSize();

  // ===== 2. 渲染场景内容 =====
  if (m_framebuffer && m_framebuffer->IsComplete()) {
    // 显示帧缓冲内容(注意UV坐标翻转)
    ImGui::Image(m_framebuffer->GetColorAttachmentID(),
                 ImVec2(m_viewportSize.x, m_viewportSize.y),
                 ImVec2(0, 1),  // UV起点(左下角)
                 ImVec2(1, 0)   // UV终点(右上角)
    );
  }

  // ===== 3. 渲染Gizmo =====
  ImGuizmo::SetDrawlist();
  if (m_currentTransform && m_gizmoInput) {
    // 设置Gizmo操作区域
    m_gizmoInput->SetViewportRect(glm::vec2(m_viewportBounds[0].x, m_viewportBounds[0].y),
                                  m_viewportSize);

    // 更新Gizmo状态
    m_gizmoInput->Update(ImGui::GetIO().DeltaTime);
  }

  // ===== 4. 绘制界面右上角ViewManipulate =====
  DrawViewManipulate();

  ImGui::End();
  ImGui::PopStyleVar();
}

bool ViewportPanel::onEvent(Event &event)
{
  // 只在视口有焦点时处理事件
  if (!m_viewportFocused || !m_viewportHovered) {
    return false;
  }
  return handleViewportEvent(event);
}

// ===== 内部方法实现 =====

void ViewportPanel::setCamera(std::shared_ptr<Camera> camera)
{
  m_camera = std::move(camera);

  if (m_currentCameraTransform) {
    delete m_currentCameraTransform;
  }
  m_currentCameraTransform = new glm::mat4(m_camera->GetViewMatrix());

  if (m_viewportInput) {
    m_viewportInput->SetCamera(m_camera);
  }
  if (m_gizmoInput) {
    m_gizmoInput->SetCamera(m_camera);
  }
}

void ViewportPanel::setFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
  m_framebuffer = std::move(framebuffer);
}

void ViewportPanel::setCurrentTransform(glm::mat4 &transform)
{
  m_currentTransform = &transform;

  // 如果Gizmo处理器尚未创建，现在创建它
  if (!m_gizmoInput && m_inputContext) {
    m_gizmoInput = std::make_shared<GizmoInputProcessor>(m_camera, *m_currentTransform);
    m_inputContext->AddProcessor(m_gizmoInput);
  }
  // 如果已存在，则更新引用
  else if (m_gizmoInput) {
    m_gizmoInput->SetTransform(*m_currentTransform);
  }
}

void ViewportPanel::updateViewportSize()
{
  // 获取视口可用区域大小
  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  m_viewportSize = {contentSize.x, contentSize.y};

  // 计算视口在屏幕中的绝对位置
  ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
  ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();
  ImVec2 windowPos = ImGui::GetWindowPos();

  m_viewportBounds[0] = {minRegion.x + windowPos.x, minRegion.y + windowPos.y};
  m_viewportBounds[1] = {maxRegion.x + windowPos.x, maxRegion.y + windowPos.y};
}

bool ViewportPanel::handleViewportEvent(Event &event)
{
  // 实际事件处理由ModularInputContext管理
  // 这里只需要转发事件
  return m_inputContext->ProcessEvent(event);
}

void ViewportPanel::DrawViewManipulate()
{
  // 1. 准备 ViewManipulate 的绘制列表和矩形区域
  float m_viewManipulateSize = 128;
  // 将小方块放置在视口的右上角
  ImVec2 viewManipulatePos = ImVec2(
      m_viewportBounds[1].x - m_viewManipulateSize - 10.0f,  // 视口右边界 - 大小 - 边距
      m_viewportBounds[0].y + 10.0f                          // 视口上边界 + 边距
  );
  ImGuizmo::SetRect(
      viewManipulatePos.x, viewManipulatePos.y, m_viewManipulateSize, m_viewManipulateSize);

  // 2. 获取当前相机的视图矩阵和投影矩阵
  // 注意：这里使用正交投影用于ViewManipulate控件本身，不影响主视口的透视投影
  glm::mat4 view = m_camera->GetViewMatrix();
  glm::mat4 projection = m_camera->GetProjectionMatrix();
  // 为ViewManipulate创建一个正交投影
  glm::mat4 orthoProjection = glm::ortho(-0.8f, 0.8f, -0.8f, 0.8f, 0.1f, 100.f);

  // 3. 保存操作前的矩阵（用于检测是否发生变化）
  glm::mat4 oldMatrix{0.0f};

  float m_ViewMatrixBuffer[16];  // 中间缓冲区
  std::memcpy(m_ViewMatrixBuffer, glm::value_ptr(view), 16 * sizeof(float));

  oldMatrix = *m_currentCameraTransform;

  // 4. 调用 ViewManipulate 函数
  // 这个函数会修改 m_viewManipulateMatrix
  ImGuizmo::ViewManipulate(glm::value_ptr(*m_currentCameraTransform),  // 被操作的矩阵
                           m_camera->GetDistance(),  // 相机距离（缩放灵敏度）
                           ImVec2(viewManipulatePos.x, viewManipulatePos.y),    // 位置
                           ImVec2(m_viewManipulateSize, m_viewManipulateSize),  // 大小
                           0x10101010  // 背景色（通常设为透明或深色）
  );

  // 5. 关键步骤：检查矩阵是否被用户操作改变了
  if (memcmp(glm::value_ptr(oldMatrix),
             glm::value_ptr(*m_currentCameraTransform),
             sizeof(float) * 16) !=
      0)
  {
    // 9. 更新相机
    if (m_camera) {
      m_camera->SetViewMatrix(*m_currentCameraTransform);
    }

    // 标记为已处理，防止其他输入干扰
    // event.handled = true; // 如果在事件回调中，可能需要这个
  }
}
};  // namespace mite