#include "blend_stage.h"
#include "basic_shader/shader_cache.h"
#include "render_opengl/opengl_command.h"

namespace mite {

BlendStage::BlendStage() : RenderStage("BlendStage")
{
  SetupBlendRenderState();
  m_Logger->info("BlendStage created");
}

BlendStage::~BlendStage()
{
  m_Logger->info("BlendStage destroyed");
}

void BlendStage::Initialize(RenderContext &context)
{
  if (m_Initialized) {
    m_Logger->warn("BlendStage already initialized");
    return;
  }

  // 创建最终输出的Framebuffer
  CreateBlendFramebuffer();

  m_Initialized = true;
  m_Logger->info("BlendStage initialization completed");
}

void BlendStage::Execute(RenderContext &context)
{
  if (!m_Initialized) {
    m_Logger->warn("BlendStage executed but not properly initialized");
    return;
  }

  if (!context.IsValid()) {
    m_Logger->warn("BlendStage executed with invalid context");
    return;
  }

  // 从上下文获取BlendStage的着色器
  auto blendShader = context.GetStageShader("BlendStage");
  if (!blendShader) {
    m_Logger->error("BlendStage: No blendShader found in context for stage 'BlendStage'");
    return;
  }
  if (blendShader->GetProgramId() == 0) {
    m_Logger->error("BlendStage: blendShader from context is not properly linked");
    return;
  }

  // 验证输入纹理
  ValidateInputs(context);

  // 获取视口尺寸并验证/调整最终Framebuffer
  auto viewportSize = context.GetViewportSize();
  ValidateBlendFramebuffer(viewportSize);

  // 绑定最终Framebuffer
  RenderCommand::Get().BindFrameBuffer(m_BlendFBO);

  // 绑定默认纯黑的环境光纹理，避免Layout绑定点悬空
  RenderCommand::Get().BindDefaultEnvironmentMap();

  // 清除输出目标
  RenderCommand::Get().Clear(
      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);

  // 设置混合渲染状态
  RenderCommand::Get().SetRenderState(m_BlendState);

  // 绑定着色器
  RenderCommand::Get().BindShader(blendShader);

  // 绑定输入纹理
  BindInputTextures(context);

  // 渲染全屏四边形
  RenderCommand::Get().DrawFullScreenQuad();

  // 解绑资源
  RenderCommand::Get().UnbindShader(blendShader);
  RenderCommand::Get().UnbindFrameBuffer();

  // 将最终输出纹理存储到上下文供后续使用
  RuntimeTexturePtr blendTexture = m_BlendFBO->GetColorAttachment(0);
  if (blendTexture && blendTexture->IsValid()) {
    context.SetRenderTarget("Forward_Blend", blendTexture);

    // 发布纹理完成事件
    RenderCommand::Get().PublishEventRuntimeTextureFinished(blendTexture, "Forward_Blend");

    m_Logger->trace("Stored blend output to context");
  }

  m_Logger->trace("Blend stage completed");

  // 调试专用：当前阶段提交完毕后直接执行
  RenderCommand::Get().Flush();
}

void BlendStage::Shutdown()
{
  // 清理Framebuffer
  if (m_BlendFBO) {
    m_BlendFBO.reset();
  }

  m_Initialized = false;
  m_Logger->info("BlendStage shutdown completed");
}

void BlendStage::CreateBlendFramebuffer()
{
  // 配置Framebuffer规格
  FrameBufferSpec spec;
  spec.samples = 1;

  // 颜色附件配置
  FrameBufferAttachmentSpec colorSpec;
  colorSpec.type = RuntimeTextureType::Forward_Blend;
  colorSpec.internalFormat = TextureFormat::RGBA16F;
  colorSpec.generateMipmaps = false;
  spec.attachments.push_back(colorSpec);

  //// 创建深度附件（延迟光照应当无需深度附件）
  //FrameBufferAttachmentSpec depthAttachment;
  //depthAttachment.type = RuntimeTextureType::Depth;
  //depthAttachment.internalFormat = TextureFormat::DEPTH_COMPONENT16;
  //depthAttachment.generateMipmaps = false;
  //spec.attachments.push_back(depthAttachment);

  // 创建Framebuffer
  m_BlendFBO = std::make_shared<FrameBuffer>(spec);

  if (!m_BlendFBO->IsComplete()) {
    m_Logger->error("Failed to create complete blend framebuffer");
    return;
  }

  m_Logger->info("Created blend output framebuffer");
}

void BlendStage::SetupBlendRenderState()
{
  m_BlendState = std::make_shared<OpenGLRenderState>();

  // 混合阶段：不需要深度测试，不需要混合（混合在着色器中完成）
  m_BlendState->depthTest = false;
  m_BlendState->depthWrite = false;
  m_BlendState->blend = false;  // 着色器内部完成混合，不需要OpenGL混合
  m_BlendState->cullFace = false;
  m_BlendState->colorWriteR = true;
  m_BlendState->colorWriteG = true;
  m_BlendState->colorWriteB = true;
  m_BlendState->colorWriteA = true;
}

void BlendStage::BindInputTextures(RenderContext &context)
{
  // 绑定Deferred Lighting纹理
  RuntimeTexturePtr deferredTexture = context.GetRenderTarget("Deferred_Lighting_Combined");
  if (deferredTexture && deferredTexture->IsValid()) {
    RenderCommand::Get().BindRuntimeTexture(RuntimeTextureType::Lighting_Combined,
                                            deferredTexture->GetHandle(),
                                            TextureTarget::TEXTURE_2D);

    m_Logger->trace("Bound deferred lighting texture");
  }
  else {
    m_Logger->warn("Missing deferred lighting texture");
  }

  // 绑定Forward半透明纹理
  RuntimeTexturePtr forwardTexture = context.GetRenderTarget("Forward_Transparent");
  if (forwardTexture && forwardTexture->IsValid()) {
    RenderCommand::Get().BindRuntimeTexture(RuntimeTextureType::Forward_Transparent,
                                            forwardTexture->GetHandle(),
                                            TextureTarget::TEXTURE_2D);

    m_Logger->trace("Bound forward transparent texture");
  }
  else {
    m_Logger->warn("Missing forward transparent texture");
  }
}

void BlendStage::ValidateInputs(RenderContext &context) const
{
  // 验证必要的输入纹理
  auto deferredTexture = context.GetRenderTarget("Deferred_Lighting_Combined");
  if (!deferredTexture || !deferredTexture->IsValid()) {
    m_Logger->error("Missing deferred lighting texture");
    throw std::runtime_error("Blend stage missing deferred lighting texture");
  }

  auto forwardTexture = context.GetRenderTarget("Forward_Transparent");
  if (!forwardTexture || !forwardTexture->IsValid()) {
    m_Logger->warn("Missing forward transparent texture - will skip blending");
  }
}

void BlendStage::ValidateBlendFramebuffer(const glm::vec2 &viewportSize)
{
  if (!m_BlendFBO) {
    m_Logger->error("Invalid blend framebuffer");
    return;
  }

  auto currentSize = m_BlendFBO->GetSize();
  if (currentSize != viewportSize) {
    m_Logger->info("Resizing blend FBO from {}x{} to {}x{}",
                   currentSize.x,
                   currentSize.y,
                   viewportSize.x,
                   viewportSize.y);
    m_BlendFBO->Resize(static_cast<uint32_t>(glm::max(viewportSize.x, 0.0f)),
                       static_cast<uint32_t>(glm::max(viewportSize.y, 0.0f)));

    if (!m_BlendFBO->IsComplete()) {
      m_Logger->error("Failed to resize blend FBO");
      throw std::runtime_error("Blend FBO resize failed");
    }
  }
}

}  // namespace mite
