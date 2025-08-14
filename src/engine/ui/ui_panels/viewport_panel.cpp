#include "viewport_panel.h"
#include "imgui.h"
#include "GLFW/glfw3.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &title) : UIPanel(title)
{
  // 初始化时创建默认相机
  m_camera = std::make_shared<Camera>();
}

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
  m_inputContext->SetBlockInput(true);

  // 初始化视口导航处理器
  m_viewportInput = std::make_shared<ViewportInputProcessor>(m_camera, GLFW_MOUSE_BUTTON_RIGHT);
  m_inputContext->AddProcessor(m_viewportInput);

  // 初始化Gizmo处理器
  if (m_currentTransform) {
    m_gizmoInput = std::make_shared<GizmoInputProcessor>(m_camera, *m_currentTransform);
    m_inputContext->AddProcessor(m_gizmoInput);
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
  if (m_framebuffer) {
    // 显示帧缓冲内容(注意UV坐标翻转)
    ImGui::Image(m_framebuffer->getColorAttachmentRendererID(),
                 ImVec2(m_viewportSize.x, m_viewportSize.y),
                 ImVec2(0, 1),  // UV起点(左下角)
                 ImVec2(1, 0)   // UV终点(右上角)
    );
  }

  // ===== 3. 渲染Gizmo =====
  if (m_currentTransform && m_gizmoInput) {
    // 设置Gizmo操作区域
    m_gizmoInput->SetViewportRect(glm::vec2(m_viewportBounds[0].x, m_viewportBounds[0].y),
                                  m_viewportSize);

    // 更新Gizmo状态
    m_gizmoInput->Update(ImGui::GetIO().DeltaTime);
  }

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
  if (m_viewportInput) {
    m_viewportInput->SetCamera(m_camera);
  }
}

void ViewportPanel::setFramebuffer(std::shared_ptr<FakeFrameBuffer> framebuffer)
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

};
