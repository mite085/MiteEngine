#include "viewport_panel.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "ImGuizmo.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &title) : UIPanel(title) {}

ViewportPanel::~ViewportPanel()
{
  // 确保输入上下文已注销
  if (m_InputContext) {
    Input::PopContext();
  }
}

void ViewportPanel::onAttach()
{
  // 创建模块化输入上下文
  m_InputContext = std::make_shared<ModularInputContext>("Viewport");
  m_InputContext->SetBlockInput(false);

  // 初始化视口导航处理器
  if (m_Camera) {
    m_ViewportInput = std::make_shared<ViewportInputProcessor>(m_Camera, GLFW_MOUSE_BUTTON_RIGHT);
    m_InputContext->AddProcessor(m_ViewportInput);
  }

  // 注册输入上下文
  Input::PushContext(m_InputContext);
}

void ViewportPanel::onDetach()
{
  // 移除输入上下文
  Input::PopContext();
  m_InputContext.reset();
}

void ViewportPanel::onUpdate(float deltaTime)
{
  // 更新视口输入状态
  if (m_ViewportInput) {
    m_ViewportInput->SetViewportHovered(m_ViewportHovered);
    m_ViewportInput->SetViewportFocused(m_ViewportFocused);
  }

  // 更新相机导航(持续移动等)
  if (m_ViewportInput && m_ViewportFocused && m_ViewportHovered) {
    m_ViewportInput->UpdateCameraTransform(deltaTime);
  }
}

void ViewportPanel::onRender()
{
  // 设置视口窗口样式(无内边距)
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin(m_Title.c_str(), &m_Visible);

  // ===== 1. 更新视口状态 =====
  m_ViewportFocused = ImGui::IsWindowFocused();
  m_ViewportHovered = ImGui::IsWindowHovered();
  updateViewportSize();

  // ===== 2. 渲染场景内容 =====
  if (m_Framebuffer && m_Framebuffer->IsComplete()) {
    // 显示帧缓冲内容(注意UV坐标翻转)
    ImGui::Image(m_Framebuffer->GetColorAttachmentID(),
                 ImVec2(m_ViewportSize.x, m_ViewportSize.y),
                 ImVec2(0, 1),  // UV起点(左下角)
                 ImVec2(1, 0)   // UV终点(右上角)
    );
  }

  // ===== 3. 渲染Gizmo =====
  ImGuizmo::SetDrawlist();
  if (m_CurrentTransform && m_GizmoInput) {
    // 设置Gizmo操作区域
    m_GizmoInput->SetViewportRect(glm::vec2(m_ViewportBounds[0].x, m_ViewportBounds[0].y),
                                  m_ViewportSize);

    // 更新Gizmo状态
    m_GizmoInput->Update(ImGui::GetIO().DeltaTime);
  }

  // ===== 4. 绘制界面右上角ViewManipulate =====
  DrawViewManipulate();

  ImGui::End();
  ImGui::PopStyleVar();
}

bool ViewportPanel::onEvent(Event &event)
{
  // 只在视口有焦点时处理事件
  if (!m_ViewportFocused || !m_ViewportHovered) {
    return false;
  }
  return handleViewportEvent(event);
}

// ===== 内部方法实现 =====

void ViewportPanel::setCamera(std::shared_ptr<Camera> camera)
{
  m_Camera = std::move(camera);

  if (m_ViewportInput) {
    m_ViewportInput->SetCamera(m_Camera);
  }
  if (m_GizmoInput) {
    m_GizmoInput->SetCamera(m_Camera);
  }
}

void ViewportPanel::setFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
  m_Framebuffer = std::move(framebuffer);
}

void ViewportPanel::setCurrentTransform(glm::mat4 &transform)
{
  m_CurrentTransform = &transform;

  // 如果Gizmo处理器尚未创建，现在创建它
  if (!m_GizmoInput && m_InputContext) {
    m_GizmoInput = std::make_shared<GizmoInputProcessor>(m_Camera, *m_CurrentTransform);
    m_InputContext->AddProcessor(m_GizmoInput);
  }
  // 如果已存在，则更新引用
  else if (m_GizmoInput) {
    m_GizmoInput->SetTransform(*m_CurrentTransform);
  }
}

void ViewportPanel::updateViewportSize()
{
  // 获取视口可用区域大小
  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  m_ViewportSize = {contentSize.x, contentSize.y};

  // 计算视口在屏幕中的绝对位置
  ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
  ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();
  ImVec2 windowPos = ImGui::GetWindowPos();

  m_ViewportBounds[0] = {minRegion.x + windowPos.x, minRegion.y + windowPos.y};
  m_ViewportBounds[1] = {maxRegion.x + windowPos.x, maxRegion.y + windowPos.y};
}

bool ViewportPanel::handleViewportEvent(Event &event)
{
  // 实际事件处理由ModularInputContext管理
  return true;
}

void ViewportPanel::DrawViewManipulate()
{
  // 1. 准备 ViewManipulate 的绘制列表和矩形区域
  float m_viewManipulateSize = 128;
  // 将小方块放置在视口的右上角
  ImVec2 viewManipulatePos = ImVec2(
      m_ViewportBounds[1].x - m_viewManipulateSize - 10.0f,  // 视口右边界 - 大小 - 边距
      m_ViewportBounds[0].y + 10.0f                          // 视口上边界 + 边距
  );
  ImGuizmo::SetRect(
      viewManipulatePos.x, viewManipulatePos.y, m_viewManipulateSize, m_viewManipulateSize);

  // 2. 获取当前相机的视图矩阵和投影矩阵
  m_CurrentCameraViewTransform = m_Camera->GetViewMatrix();
  glm::mat4 projection = m_Camera->GetProjectionMatrix();

  // 3. 保存操作前的矩阵（用于检测是否发生变化）
  glm::mat4 oldMatrix = m_CurrentCameraViewTransform;

  // 4. 调用 ViewManipulate 函数
  // 这个函数会修改 m_viewManipulateMatrix
  ImGuizmo::ViewManipulate(glm::value_ptr(m_CurrentCameraViewTransform),  // 被操作的矩阵
                           m_Camera->GetDistance(),  // 相机距离（缩放灵敏度）
                           ImVec2(viewManipulatePos.x, viewManipulatePos.y),    // 位置
                           ImVec2(m_viewManipulateSize, m_viewManipulateSize),  // 大小
                           0x00000000  // 背景色（通常设为透明或深色）
  );

  // 5. 关键步骤：检查矩阵是否被用户操作改变了
  if (memcmp(glm::value_ptr(oldMatrix),
             glm::value_ptr(m_CurrentCameraViewTransform),
             sizeof(float) * 16) != 0)
  {
    // 6. 更新相机
    if (m_Camera) {
      m_Camera->SetViewMatrix(m_CurrentCameraViewTransform);
    }
  }
}
};  // namespace mite