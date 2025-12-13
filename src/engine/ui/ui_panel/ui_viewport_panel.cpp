#include "ui_viewport_panel.h"
#include "input/input_manager.h"

namespace mite {
ViewportPanel::ViewportPanel(SceneView &sceneView, const std::string &name)
    : UIPanel(name), m_SceneView(sceneView), m_PanelSize(glm::vec2(1280, 720))
{
  m_EventSubscriptions.SubscribeImmediate<RuntimeTextureFinishedEvent>(
      BIND_DISPATCH_FN(OnRenderFinished));
  m_EventSubscriptions.SubscribeImmediate<DisplayTextureTypeChangedEvent>(
      BIND_DISPATCH_FN(OnDisplayTextureTypeChanged)); 

  // 初始化面板属性
  InitializePanelProps();

  // 初始化ImageProps图像属性
  m_ImageProps = ImageProps();
  m_ImageProps.visible = true;
  m_ImageProps.enabled = true;
  m_ImageProps.uv0 = glm::vec2(0.0f, 1.0f);  // 翻转Y轴
  m_ImageProps.uv1 = glm::vec2(1.0f, 0.0f);

  // 初始化Overlay
  m_GizmoOverlay = std::make_unique<GizmoOverlay>();
  m_GizmoOverlayContext = OverlayContext();

  // 创建输入上下文，并注册
  m_InputContext = std::make_shared<ViewportInputContext>("Viewport Input Context");
  InputManager::Get().PushContext(m_InputContext);

  LOG_DEBUG("Created ViewportPanel: {}", name);
}
void ViewportPanel::Update(float deltaTime)
{
  UpdateImageProps();
  UpdateOverlayContext();
  UpdateInputContext(deltaTime, m_GizmoOverlay->IsUsing());
}

void ViewportPanel::Render()
{
  try {
    // 获取当前内容区域尺寸
    glm::vec2 panelPos = m_Renderer.GetPanelPos();
    glm::vec2 panelSize = m_Renderer.GetPanelSize();

    // 确保尺寸都是正数。否则类型转换会有问题（当ViewPort折叠起来的时候，size.y为-16）
    if (panelSize.x <= 0 || panelSize.y <= 0) {
      // 面板收起或尺寸无效时跳过渲染
      return;
    }

    // 处理边界尺寸变化
    UpdatePanelBorder(panelPos, panelSize);

    // 获取当前显示缓冲
    if (m_DisplayTexture) {
      // 渲染图像
      m_Renderer.RenderImage(m_ImageProps);

      // 绘制Gizmo Overlay
      m_GizmoOverlay->Render(m_GizmoOverlayContext);

      // Gizmo完成之后应用变换
      m_InputContext->Apply(m_GizmoOverlayContext.cameraTransform);
    }
    else {
      // FrameBuffer未就绪时的占位显示
      LabelProps placeholderProps;
      placeholderProps.visible = true;
      placeholderProps.translationKey = "FrameBuffer Not Ready";
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
  props.resizable = true;      // 可调整大小（关键：允许拖拽调整）
  props.scrollable = false;    // 视口不需要滚动条
  props.collapsed = false;     // 不折叠标题
  props.bringToFront = false;  // 不强制最上层显示

  // 设置合理的尺寸限制
  props.minSize = glm::vec2(0, 0);        // 最小尺寸
  props.maxSize = glm::vec2(3840, 2160);  // 最大4K分辨率
}
void ViewportPanel::UpdatePanelBorder(const glm::vec2 &newPos, const glm::vec2 &newSize)
{
  // 更新Panel位置
  if (newPos != m_PanelPos) {
    m_PanelPos = newPos;
  }

  // 更新Panel尺寸
  if (newSize != m_PanelSize) {
    m_PanelSize = newSize;

    // 发布Resize事件，Pipeline和SceneView接收事件后调整FBO尺寸与相机宽高比
    EventBus::Publish<ViewportResizeEvent>(ViewportResizeEvent(m_PanelSize));

    LOG_DEBUG("ViewportPanel size changed to {}x{}", newSize.x, newSize.y);
  }
}
void ViewportPanel::UpdateOverlayContext()
{
  m_GizmoOverlayContext.mousePos = m_Renderer.GetMousePos();
  // 更新Overlay上下文视口信息
  m_GizmoOverlayContext.viewportPos = m_PanelPos;
  m_GizmoOverlayContext.viewportSize = m_PanelSize;
  m_GizmoOverlayContext.contentPos = m_PanelPos;  // 内容位置和尺寸与视口位置尺寸保持一致即可
  m_GizmoOverlayContext.contentSize = m_PanelSize;

  // 更新Overlay上下文矩阵信息
  m_GizmoOverlayContext.cameraTransform = m_SceneView.GetCameraInstance()->GetCameraTransform();
  m_GizmoOverlayContext.cameraProjection = m_SceneView.GetCameraInstance()->GetProjectionMatrix();
  m_GizmoOverlayContext.isModelSelected = m_SceneView.IsPicked();
  m_GizmoOverlayContext.modelTransform = m_SceneView.GetPickedWorldTransform();
}
void ViewportPanel::UpdateImageProps()
{
  if (m_DisplayTexture) {
    // 更新ImageProps句柄
    m_ImageProps.textureId = m_DisplayTexture->GetHandle().apiHandle;

    // 设置图像尺寸为面板内容尺寸
    m_ImageProps.size = m_PanelSize;
    m_ImageProps.visible = IsVisible();
    m_ImageProps.enabled = IsEnabled();
  }
}
void ViewportPanel::UpdateInputContext(float deltatime, bool gizmoUsing)
{
  // 更新聚焦信息和视口尺寸
  m_InputContext->SetViewportFocus(m_PanelProps.isFocused);
  m_InputContext->SetViewportHovered(m_PanelProps.isHovered);
  m_InputContext->SetViewportRect(m_PanelPos, m_PanelSize);

  // 更新输入上下文
  m_InputContext->Update(deltatime, gizmoUsing);
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

void ViewportPanel::OnDisplayTextureTypeChanged(DisplayTextureTypeChangedEvent &event) 
{
  if (event.GetDisplayTextureType() != m_DisplayTextureType) {
    // 执行更新操作
    m_DisplayTextureType = event.GetDisplayTextureType();

    // 已处理，继续传播
    event.SetResult(EventResult::Handled);
  }
}

}  // namespace mite