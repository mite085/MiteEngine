#include "ui_editor_viewport_panel.h"
#include "ui_core/ui_render.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &title)
    : UIPanel(title, UILayout::LayoutType::Vertical, UILayout::Alignment::TopLeft)
{
  // 设置面板属性
  SetDraggable(false);
  SetResizable(true);
  SetSize({800, 600});  // 默认尺寸
}
ViewportPanel::~ViewportPanel()
{
  // 清理资源
}
void ViewportPanel::Render()
{
  if (!IsVisible())
    return;

  // 应用布局
  if (m_LayoutDirty) {
    ApplyLayout();
    m_LayoutDirty = false;
  }
   
  // 渲染面板背景
  GroupProps groupProps;
  groupProps.translationKey = "viewport.panel";
  groupProps.fallbackText = "Viewport";
  groupProps.size = GetSize();
  groupProps.position = GetPosition();

  UIRender::Get().RenderGroup(groupProps, [this]() {
    RenderToolbar();
    RenderViewportContent();

    if (m_ShowOverlay) {
      RenderOverlay();
    }
  });

  // 处理输入
  if (m_Interactive) {
    HandleInput();
  }
}
void ViewportPanel::RenderViewportContent()
{
  // 计算视口区域（考虑工具栏高度）
  glm::vec2 contentPos = GetPosition() + glm::vec2(0, 30);  // 工具栏高度
  glm::vec2 contentSize = GetSize() - glm::vec2(0, 30);

  // 保持宽高比
  if (m_AspectRatio > 0) {
    float targetWidth = contentSize.y * m_AspectRatio;
    if (targetWidth > contentSize.x) {
      contentSize.y = contentSize.x / m_AspectRatio;
    }
    else {
      contentSize.x = targetWidth;
    }
  }

  m_ViewportSize = contentSize;
  m_ViewportPosition = contentPos + (GetSize() - glm::vec2(0, 30) - contentSize) * 0.5f;

  // 渲染帧缓冲区内容
  ImageProps imageProps;
  imageProps.textureId = static_cast<uintptr_t>(m_FrameBuffer->GetColorAttachmentID());
  imageProps.position = m_ViewportPosition;
  imageProps.size = m_ViewportSize;
  imageProps.translationKey = "viewport.content";

  UIRender::Get().RenderImage(imageProps);
}
void ViewportPanel::RenderToolbar()
{
  // 工具栏布局
  GroupProps toolbarProps;
  toolbarProps.translationKey = "viewport.toolbar";
  toolbarProps.size = {GetSize().x, 30};
  toolbarProps.position = GetPosition();

  UIRender::Get().RenderGroup(toolbarProps, [this]() {
    // 工具栏按钮
    ButtonProps toggleOverlayBtn;
    toggleOverlayBtn.translationKey = m_ShowOverlay ? "viewport.hide_overlay" :
                                                      "viewport.show_overlay";
    toggleOverlayBtn.fallbackText = m_ShowOverlay ? "Hide Overlay" : "Show Overlay";
    toggleOverlayBtn.size = {100, 25};

    if (UIRender::Get().RenderButton(toggleOverlayBtn)) {
      m_ShowOverlay = !m_ShowOverlay;
    }

    UIRender::Get().SetSameLine();

    // 分辨率显示
    LabelProps resolutionLabel;
    resolutionLabel.translationKey = "viewport.resolution";
    resolutionLabel.fallbackText = "Resolution: ";
    UIRender::Get().RenderLabel(resolutionLabel);

    UIRender::Get().SetSameLine();

    LabelProps resolutionValue;
    resolutionValue.fallbackText = std::to_string(static_cast<int>(m_ViewportSize.x)) + "x" +
                                   std::to_string(static_cast<int>(m_ViewportSize.y));
    UIRender::Get().RenderLabel(resolutionValue);
  });
}
void ViewportPanel::RenderOverlay()
{
  // 渲染统计信息覆盖层
  LabelProps statsLabel;
  statsLabel.position = m_ViewportPosition + glm::vec2(10, 10);
  statsLabel.translationKey = "viewport.stats";
  statsLabel.fallbackText = "FPS: 60 | Triangles: 1000";
  UIRender::Get().RenderLabel(statsLabel);

  // 渲染安全框
  if (m_AspectRatio > 0) {
    // 计算安全框位置和尺寸
    glm::vec2 safeAreaSize = m_ViewportSize * 0.9f;
    glm::vec2 safeAreaPos = m_ViewportPosition + (m_ViewportSize - safeAreaSize) * 0.5f;

    // 渲染安全框边框
    // 可以使用ImGui的绘制列表功能
  }
}
void ViewportPanel::HandleInput()
{
  // 检查鼠标是否在视口内
  auto inputSystem = InputSystem::GetInstance();
  glm::vec2 mousePos = inputSystem->GetMousePosition();

  m_IsHovered = (mousePos.x >= m_ViewportPosition.x &&
                 mousePos.x <= m_ViewportPosition.x + m_ViewportSize.x &&
                 mousePos.y >= m_ViewportPosition.y &&
                 mousePos.y <= m_ViewportPosition.y + m_ViewportSize.y);

  // 处理视口内的输入事件
  if (m_IsHovered) {
    // 摄像机控制、对象选择等
    if (inputSystem->IsMouseButtonPressed(MouseButton::Right)) {
      // 启动摄像机旋转
    }

    if (inputSystem->IsMouseButtonPressed(MouseButton::Left)) {
      // 对象选择
    }
  }
}
// ==================== 帧缓冲区管理 ====================
void ViewportPanel::SetFrameBuffer(std::shared_ptr<FrameBuffer> frameBuffer)
{
  m_FrameBuffer = frameBuffer;
  MarkLayoutDirty();
}
std::shared_ptr<FrameBuffer> ViewportPanel::GetFrameBuffer() const
{
  return m_FrameBuffer;
}
// ==================== 视口控制 ====================
void ViewportPanel::SetAspectRatio(float aspectRatio)
{
  m_AspectRatio = aspectRatio;
  MarkLayoutDirty();
}
float ViewportPanel::GetAspectRatio() const
{
  return m_AspectRatio;
}
glm::vec2 ViewportPanel::GetViewportSize() const
{
  return m_ViewportSize;
}
// ==================== 交互控制 ====================
void ViewportPanel::SetInteractive(bool interactive)
{
  m_Interactive = interactive;
}
bool ViewportPanel::IsInteractive() const
{
  return m_Interactive;
}
// ==================== 覆盖层控制 ====================
void ViewportPanel::ShowOverlay(bool show)
{
  m_ShowOverlay = show;
}
bool ViewportPanel::IsOverlayVisible() const
{
  return m_ShowOverlay;
}


}  // namespace mite