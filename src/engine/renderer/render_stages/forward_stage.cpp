#include "forward_stage.h"

namespace mite {
ForwardStage::ForwardStage() : RenderStage("ForwardStage")
{
  // 不透明物体状态
  m_OpaqueState.depthTest = true;
  m_OpaqueState.depthWrite = true;
  m_OpaqueState.blend = false;
  m_OpaqueState.cullFace = true;
  m_OpaqueState.wireframe = false;

  // Alpha测试物体状态
  m_AlphaTestState.depthTest = true;
  m_AlphaTestState.depthWrite = true;
  m_AlphaTestState.blend = false;
  m_AlphaTestState.cullFace = true;
  m_AlphaTestState.wireframe = false;

  // 透明物体状态
  m_TransparentState.depthTest = true;
  m_TransparentState.depthWrite = false;  // 透明物体不写入深度
  m_TransparentState.blend = true;        // 启用混合
  m_TransparentState.cullFace = true;
  m_TransparentState.wireframe = false;

  // 自定义物体状态（默认与不透明相同）
  m_CustomState = m_OpaqueState;
  m_Logger->info("ForwardStage initialized with complete render state configurations");
}

ForwardStage::~ForwardStage()
{
  m_Logger->info("ForwardStage destroyed");
}

void ForwardStage::Initialize()
{
  // 创建FrameBuffer规格
  FrameBufferSpec spec;
  spec.attachments = {
      {RuntimeTextureType::RenderTarget, TextureFormat::RGBA8},  // 颜色附件
      {RuntimeTextureType::Depth, TextureFormat::DEPTH_COMPONENT24}};  // 深度附件

  // 创建FrameBuffer用于存储数据
  m_ForwardFrameBuffer = std::make_shared<FrameBuffer>(spec);

  if (!m_ForwardFrameBuffer->IsComplete()) {
    m_Logger->error("Failed to create complete framebuffers for forward rendering");
    throw std::runtime_error("Framebuffers are incomplete");
  }

  m_Logger->info("ForwardStage initialization completed");
}

void ForwardStage::Execute(RenderContext &context)
{
  // 验证上下文有效性
  if (!context.IsValid()) {
    m_Logger->warn("ForwardStage executed with invalid context");
    return;
  }

  // 获取渲染队列
  auto renderQueue = context.GetRenderQueue();
  if (!renderQueue) {
    m_Logger->warn("ForwardStage: No render queue available");
    return;
  }

  // 获取上下文记录的帧缓冲尺寸
  glm::uvec2 viewportSize = context.GetViewportSize();

  // 若与帧缓冲尺寸不匹配，则执行Resize（直接执行即可，无需提交给Commit队列）
  if (m_ForwardFrameBuffer->GetSize() != viewportSize) {
    m_ForwardFrameBuffer->Resize(viewportSize.x, viewportSize.y);
  }

  // 绑定前向渲染的FrameBuffer
  RenderCommand::Get().BindFrameBuffer(m_ForwardFrameBuffer);

  // stage开始渲染之前绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetCameraInstance());

  // 按顺序渲染各个队列
  RenderOpaqueQueue(context);
  RenderAlphaTestQueue(context);
  RenderTransparentQueue(context);
  RenderCustomQueue(context);

  // 解绑FrameBuffer，恢复默认
  RenderCommand::Get().UnbindFrameBuffer();
}

void ForwardStage::Shutdown()
{
  m_Logger->info("ForwardStage shutdown completed");
}

void ForwardStage::RenderOpaqueQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::Opaque);

  if (items.empty()) {
    m_LastFrameOpaqueCount = 0;
    return;
  }

  // 设置不透明物体渲染状态
  SetupRenderStateForQueue(RenderQueue::QueueType::Opaque);

  // 渲染所有不透明物体
  size_t renderedCount = 0;
  for (const auto &item : items) {
    if (ValidateRenderableItem(item)) {
      RenderCommand::Get().Submit(item);
      renderedCount++;
    }
  }

  m_LastFrameOpaqueCount = renderedCount;
  // m_Logger->trace("Rendered {} opaque objects", renderedCount);
}

void ForwardStage::RenderAlphaTestQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::AlphaTest);

  if (items.empty()) {
    m_LastFrameAlphaTestCount = 0;
    return;
  }

  // 设置Alpha测试物体渲染状态
  SetupRenderStateForQueue(RenderQueue::QueueType::AlphaTest);

  // 渲染所有Alpha测试物体
  size_t renderedCount = 0;
  for (const auto &item : items) {
    if (ValidateRenderableItem(item)) {
      RenderCommand::Get().Submit(item);
      renderedCount++;
    }
  }

  m_LastFrameAlphaTestCount = renderedCount;
  m_Logger->trace("Rendered {} alpha-test objects", renderedCount);
}

void ForwardStage::RenderTransparentQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::Transparent);

  if (items.empty()) {
    m_LastFrameTransparentCount = 0;
    return;
  }

  // 设置透明物体渲染状态
  SetupRenderStateForQueue(RenderQueue::QueueType::Transparent);

  // 渲染所有透明物体（可按距离排序优化）
  size_t renderedCount = 0;
  for (const auto &item : items) {
    if (ValidateRenderableItem(item)) {
      RenderCommand::Get().Submit(item);
      renderedCount++;
    }
  }

  m_LastFrameTransparentCount = renderedCount;
  m_Logger->trace("Rendered {} transparent objects", renderedCount);
}

void ForwardStage::RenderCustomQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::Custom);

  if (items.empty()) {
    m_LastFrameCustomCount = 0;
    return;
  }

  // 设置自定义物体渲染状态
  SetupRenderStateForQueue(RenderQueue::QueueType::Custom);

  // 渲染所有自定义物体
  size_t renderedCount = 0;
  for (const auto &item : items) {
    if (ValidateRenderableItem(item)) {
      RenderCommand::Get().Submit(item);
      renderedCount++;
    }
  }

  m_LastFrameCustomCount = renderedCount;
  m_Logger->trace("Rendered {} custom objects", renderedCount);
}

bool ForwardStage::ValidateRenderableItem(const RenderableItem &item) const
{
  // 验证材质和Shader
  if (!item.material) {
    m_Logger->warn("Renderable item has no material");
    return false;
  }

  auto shader = item.material->GetShader();
  if (!shader || shader->GetHandle().programId == 0) {
    m_Logger->warn("Renderable item has invalid shader");
    return false;
  }

  // 验证网格数据
  if (item.mesh.GetModelHandle().vertexArray == 0) {
    m_Logger->warn("Renderable item has invalid mesh");
    return false;
  }

  return true;
}

void ForwardStage::SetupRenderStateForQueue(RenderQueue::QueueType queueType)
{
  // 根据队列类型设置渲染状态
  switch (queueType) {
    case RenderQueue::QueueType::Opaque:
      RenderCommand::Get().SetRenderState(m_OpaqueState);
      break;
    case RenderQueue::QueueType::AlphaTest:
      RenderCommand::Get().SetRenderState(m_AlphaTestState);
      break;
    case RenderQueue::QueueType::Transparent:
      RenderCommand::Get().SetRenderState(m_TransparentState);
      break;
    case RenderQueue::QueueType::Custom:
      RenderCommand::Get().SetRenderState(m_CustomState);
      break;
    default:
      m_Logger->warn("Unknown queue type for render state setup");
      RenderCommand::Get().SetRenderState(m_OpaqueState);
      break;
  }
}
}  // namespace mite