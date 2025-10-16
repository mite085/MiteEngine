#include "ui_viewport_panel.h"
#include "basic_event/render_event.h"
#include "imgui.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &name)
    : UIPanel(name), m_CurrentSize(glm::uvec2(1280, 720))
{
  m_EventSubscriptions.SubscribeImmediate<RuntimeTextureFinishedEvent>(
      BIND_DISPATCH_FN(OnRenderFinished));

  // 初始化面板属性
  InitializePanelProps();

  // 初始化ImageProps
  m_ImageProps.elementId = UUIDGenerator::Generate();
  m_ImageProps.visible = true;
  m_ImageProps.enabled = true;
  m_ImageProps.uv0 = glm::vec2(0.0f, 1.0f);  // 翻转Y轴
  m_ImageProps.uv1 = glm::vec2(1.0f, 0.0f);

  LOG_DEBUG("Created ViewportPanel: {}", name);
}
void ViewportPanel::Update(float deltaTime) {}
void ViewportPanel::Render()
{
  try {
    // 获取当前内容区域尺寸
    glm::vec2 contentStartPos = m_Renderer.GetCursorStartPos();
    glm::vec2 contentSize = m_Renderer.GetContentRegionAvail();

    // 确保尺寸都是正数。否则类型转换会有问题（当ViewPort折叠起来的时候，size.y为-16）
    if (contentSize.x <= 0 || contentSize.y <= 0) {
      // 面板收起或尺寸无效时跳过渲染
      return;
    }
    glm::uvec2 newSize(static_cast<uint32_t>(contentSize.x), static_cast<uint32_t>(contentSize.y));

    // 处理尺寸变化
    HandleSizeChange(newSize);

    // 获取当前显示缓冲
    if (m_DisplayTexture) {
      // 更新ImageProps句柄
      m_ImageProps.textureId = m_DisplayTexture->getHandle().apiHandle; 

      // 设置图像尺寸为面板内容尺寸
      m_ImageProps.size = newSize;
      m_ImageProps.visible = IsVisible();
      m_ImageProps.enabled = IsEnabled();

      // 渲染图像
      m_Renderer.RenderImage(m_ImageProps);
    }
    else {
      // FrameBuffer未就绪时的占位显示
      LabelProps placeholderProps;
      placeholderProps.visible = true;
      placeholderProps.fallbackText = "FrameBuffer Not Ready";
      m_Renderer.RenderLabel(placeholderProps);
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
  props.minSize = glm::vec2(0, 0);        // 最小尺寸
  props.maxSize = glm::vec2(3840, 2160);  // 最大4K分辨率
}
void ViewportPanel::HandleSizeChange(const glm::uvec2 &newSize)
{
  if (newSize == m_CurrentSize) {
    return;
  }

  // 更新当前尺寸
  m_CurrentSize = newSize;
  EventBus::Publish<ViewPortResizeEvent>(ViewPortResizeEvent(m_CurrentSize));

  // LOG_DEBUG("ViewportPanel size changed to {}x{}", newSize.x, newSize.y);
}
void ViewportPanel::OnRenderFinished(RuntimeTextureFinishedEvent &event)
{
  // 首先匹配纹理类型
  if (event.GetTextureType() != m_DisplayTextureType)
    return;

  // 然后匹配纹理标识符（若m_Identify为空则不执行匹配）
  if (!m_DisplayTextureIdentify.empty() && m_DisplayTextureIdentify != event.GetIdentify())
    return;

  // 执行更新操作
  m_DisplayTexture = event.GetTexture();

  // 已处理，继续传播
  event.SetResult(EventResult::Handled);
}
}  // namespace mite