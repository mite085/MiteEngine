#include "forward_stage.h"
#include "render_opengl/opengl_command.h"

namespace mite {
ForwardStage::ForwardStage() : RenderStage("ForwardStage")
{
  // 透明物体状态
  m_TransparentState = std::make_shared<OpenGLRenderState>();
  m_TransparentState->depthTest = true;    // 启用深度测试
  m_TransparentState->depthWrite = false;  // 但不写入深度
  m_TransparentState->blend = true;        // 启用混合
  m_TransparentState->cullFace = true;
  m_TransparentState->wireframe = false;

  m_Logger->info("ForwardStage initialized with complete render state configurations");
}

ForwardStage::~ForwardStage()
{
  m_Logger->info("ForwardStage destroyed");
}

void ForwardStage::Initialize(RenderContext &context)
{
  // 创建FrameBuffer规格
  FrameBufferSpec spec;
  spec.samples = 1;

  // 颜色附件配置 - 使用HDR格式存储光照结果
  FrameBufferAttachmentSpec colorSpec;
  colorSpec.type = RuntimeTextureType::Forward_Transparent;  // 透明物体渲染结果
  colorSpec.internalFormat = TextureFormat::RGBA16F;         // HDR输出
  colorSpec.generateMipmaps = false;
  spec.attachments.push_back(colorSpec);

  // Forward不创建深度附件，而是直接绑定GBuffer的深度附件
  // 这样才能确保半透明物体的正常绘制
  // FrameBufferAttachmentSpec depthAttachment;
  // depthAttachment.type = RuntimeTextureType::Depth;
  // depthAttachment.internalFormat = TextureFormat::DEPTH_COMPONENT16;
  // depthAttachment.generateMipmaps = false;
  // spec.attachments.push_back(depthAttachment);

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

  // 从上下文获取G-Buffer Stage的着色器
  std::shared_ptr<OpenGLShader> forwardShader = context.GetStageShader(m_Name);
  if (!forwardShader) {
    m_Logger->error("Forward Stage: No shader registered for Forward Stage in context");
    return;
  }
  if (forwardShader->GetProgramId() == 0) {
    m_Logger->error("Forward Stage: Shader from context is not properly linked");
    return;
  }

  // 获取上下文记录的帧缓冲尺寸
  glm::vec2 viewportSize = context.GetViewportSize();

  // 若与帧缓冲尺寸不匹配，则执行Resize（直接执行即可，无需提交给Commit队列）
  if (m_ForwardFrameBuffer->GetSize() != viewportSize) {
    m_ForwardFrameBuffer->Resize(static_cast<uint32_t>(glm::max(viewportSize.x, 0.0f)),
                                 static_cast<uint32_t>(glm::max(viewportSize.y, 0.0f)));
  }

  // 绑定G-Buffer的深度附件
  // （由于FBO的Resize操作会重新Invalidate，且不知道何时Resize，所以需要每帧重新绑定）
  // （这一步开销应当不是很大，在容忍范围内）
  // （待后续FBO的Resize操作无需重建全部纹理时，再将这一步放在初始化阶段）
  auto gbufferDepthTexture = context.GetRenderTarget("GBuffer_DepthAttachment");
  if (gbufferDepthTexture && gbufferDepthTexture->IsValid()) {
    m_ForwardFrameBuffer->AttachExternalDepthTexture(gbufferDepthTexture);
  }
  else {
    m_Logger->error("Failed to get GBuffer depth texture for Forward stage");
    return;
  }

  // 绑定前向渲染的FrameBuffer
  RenderCommand::Get().BindFrameBuffer(m_ForwardFrameBuffer);

  // 清除输出目标（深度附件为复用的GBuffer的，不清理深度Buffer）
  RenderCommand::Get().Clear(GL_COLOR_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);

  // 绑定着色器
  RenderCommand::Get().BindShader(forwardShader);

  // 绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetMainCameraInstance());

  // 绑定默认纯黑的环境光纹理，避免Layout绑定点悬空
  RenderCommand::Get().BindDefaultEnvironmentMap();

  // 按顺序渲染各个队列
  RenderTransparentQueue(context);

  // 解绑FrameBuffer，恢复默认
  RenderCommand::Get().UnbindFrameBuffer();

  // 将光照输出纹理存储到上下文供后续阶段使用
  RuntimeTexturePtr forwardTexture = m_ForwardFrameBuffer->GetColorAttachment(0);
  if (forwardTexture && forwardTexture->IsValid()) {
    context.SetRenderTarget("Forward_Transparent", forwardTexture);
    // 发布纹理完成事件
    RenderCommand::Get().PublishEventRuntimeTextureFinished(forwardTexture, "Forward_Transparent");
    m_Logger->trace("Stored deferred lighting output to context");
  }

  m_Logger->trace("Forward pass completed");

  // 调试专用：当前阶段提交完毕后直接执行
  RenderCommand::Get().Flush();
}

void ForwardStage::Shutdown()
{
  m_Logger->info("ForwardStage shutdown completed");
}

void ForwardStage::RenderTransparentQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  const auto &items = renderQueue->GetItems(RenderableItemType::Transparent);

  if (items.empty()) {
    m_LastFrameTransparentCount = 0;
    return;
  }

  // 设置透明物体渲染状态
  RenderCommand::Get().SetRenderState(m_TransparentState);

  // 渲染所有透明物体（可按距离排序优化）
  size_t renderedCount = 0;
  for (const auto &item : items) {
    if (ValidateRenderableItem(item)) {
      RenderCommand::Get().BindMaterialUBO(item.material);
      RenderCommand::Get().SubmitDrawCall(item.mesh);
      renderedCount++;
    }
  }

  m_LastFrameTransparentCount = renderedCount;
  m_Logger->trace("Rendered {} transparent objects", renderedCount);
}

bool ForwardStage::ValidateRenderableItem(const RenderableItem &item) const
{
  // 验证材质和Shader
  if (!item.material) {
    m_Logger->warn("Renderable item has no material");
    return false;
  }

  // 验证网格数据
  if (item.mesh->GetMesh()->GetModelHandle().vertexArray == 0) {
    m_Logger->warn("Renderable item has invalid mesh");
    return false;
  }

  return true;
}
}  // namespace mite