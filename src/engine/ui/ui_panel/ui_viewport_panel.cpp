#include "ui_viewport_panel.h"
#include "imgui.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &name,
                             CameraComponent &camera,
                             RenderPipeline &renderer)
    : UIPanel(name), m_CameraComponent(camera), m_Renderer(renderer)
{
  // 初始化面板属性
  InitializePanelProps();

  // 初始化ImageProps
  m_ImageProps.elementId = UUIDGenerator::Generate();
  m_ImageProps.visible = true;
  m_ImageProps.enabled = true;
  m_ImageProps.uv0 = glm::vec2(0.0f, 1.0f);  // 翻转Y轴
  m_ImageProps.uv1 = glm::vec2(1.0f, 0.0f);

  // 获取初始尺寸
  auto displayBuffer = m_Renderer.GetDisplayFrameBuffer();
  if (displayBuffer) {
    m_CurrentSize = displayBuffer->GetSize();
  }
  else {
    m_CurrentSize = glm::uvec2(800, 600);  // 默认尺寸
  }
  LOG_DEBUG("Created ViewportPanel: {}", name);
}
void ViewportPanel::Update(float deltaTime)
{
  // 处理延迟的尺寸调整请求
  if (m_SizeDirty && m_RequestedSize.x > 0 && m_RequestedSize.y > 0) {
    ResizeMainFrameBuffer(m_RequestedSize);
    m_SizeDirty = false;
  }
}
void ViewportPanel::Render()
{
  try {
    // 获取当前内容区域尺寸
    glm::vec2 contentSize = GetContentRegionAvail();

    // 确保尺寸都是正数。否则类型转换会有问题（当ViewPort折叠起来的时候，size.y为-16）
    if (contentSize.x <= 0 || contentSize.y <= 0) {
      // 面板收起或尺寸无效时跳过渲染
      return;
    }
    glm::uvec2 newSize(static_cast<uint32_t>(contentSize.x), static_cast<uint32_t>(contentSize.y));

    // 处理尺寸变化
    if (newSize != m_CurrentSize) {
      HandleSizeChange(newSize);
    }
    // 获取当前显示缓冲
    auto displayBuffer = m_Renderer.GetDisplayFrameBuffer();
    if (displayBuffer && displayBuffer->GetColorAttachmentID() != 0) {
      // 更新ImageProps
      UpdateImagePropsFromDisplayBuffer();

      // 设置图像尺寸为面板内容尺寸
      m_ImageProps.size = newSize;
      m_ImageProps.visible = IsVisible();
      m_ImageProps.enabled = IsEnabled();

      // 渲染图像
      GetRenderer().RenderImage(m_ImageProps);
    }
    else {
      // FrameBuffer未就绪时的占位显示
      LabelProps placeholderProps;
      placeholderProps.visible = true;
      placeholderProps.fallbackText = "FrameBuffer Not Ready";
      GetRenderer().RenderLabel(placeholderProps);
    }
  }
  catch (const std::exception &e) {
    LOG_ERROR("ViewportPanel render error: {}", e.what());
  }
}
void ViewportPanel::InitializePanelProps()
{
  auto &props = GetPanelProps();

  // 视口面板专用配置
  props.movable = true;        // 可移动
  props.resizable = true;      // 可调整大小（关键：允许拖拽调整）
  props.scrollable = false;    // 视口不需要滚动条
  props.collapsed = false;     // 不折叠标题
  props.bringToFront = false;  // 不强制最上层显示

  // 设置合理的尺寸限制
  props.minSize = glm::vec2(0, 0);     // 最小尺寸
  props.maxSize = glm::vec2(3840, 2160);  // 最大4K分辨率
}
void ViewportPanel::UpdateImagePropsFromDisplayBuffer()
{
  auto displayBuffer = m_Renderer.GetDisplayFrameBuffer();
  if (displayBuffer) {
    m_ImageProps.textureId = static_cast<uintptr_t>(displayBuffer->GetColorAttachmentID());
  }
}
void ViewportPanel::HandleSizeChange(const glm::uvec2 &newSize)
{
  // 更新当前尺寸
  m_CurrentSize = newSize;

  // 设置相机宽高比（避免画面拉伸）
  if (m_CurrentSize.y > 0) {
    float aspectRatio = static_cast<float>(m_CurrentSize.x) / static_cast<float>(m_CurrentSize.y);
    m_CameraComponent.SetAspectRatio(aspectRatio);
  }

  // 请求调整MainFrameBuffer尺寸（在Update中执行）
  m_RequestedSize = newSize;
  m_SizeDirty = true;

  // LOG_DEBUG("ViewportPanel size changed to {}x{}", newSize.x, newSize.y);
}
void ViewportPanel::ResizeMainFrameBuffer(const glm::uvec2 &newSize)
{
  // 获取主FrameBuffer并调整尺寸
  auto mainBuffer = m_Renderer.GetMainFrameBuffer();
  if (mainBuffer) {
    try {
      mainBuffer->Resize(newSize.x, newSize.y);
      // LOG_DEBUG("Resized MainFrameBuffer to {}x{}", newSize.x, newSize.y);
    }
    catch (const std::exception &e) {
      LOG_ERROR("Failed to resize MainFrameBuffer: {}", e.what());
    }
  }

  // 注意：DisplayFrameBuffer不需要手动调整，Renderer的BeginFrame()会处理双缓冲的尺寸同步
}
}  // namespace mite