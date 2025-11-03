#include "deferred_lighting_stage.h"
#include "basic_shader/gbuffer.h"
#include "basic_shader/shader_cache.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "render_opengl/opengl_command.h"

namespace mite {

DeferredLightingStage::DeferredLightingStage() : RenderStage("DeferredLightingStage")
{
  SetupLightingRenderState();
  m_Logger->info("DeferredLightingStage created");
}

DeferredLightingStage::~DeferredLightingStage()
{
  m_Logger->info("DeferredLightingStage destroyed");
}

void DeferredLightingStage::Initialize()
{
  if (m_Initialized) {
    m_Logger->warn("DeferredLightingStage already initialized");
    return;
  }

  // 创建默认尺寸的光照Framebuffer
  CreateLightingFramebuffer();

  m_Initialized = true;
  m_Logger->info("DeferredLightingStage initialization completed");
}

void DeferredLightingStage::Execute(RenderContext &context)
{
  if (!m_Initialized) {
    m_Logger->warn("DeferredLightingStage executed but not properly initialized");
    return;
  }

  if (!context.IsValid()) {
    m_Logger->warn("DeferredLightingStage executed with invalid context");
    return;
  }

  // 从上下文获取DeferredLightingStage的着色器
  auto lightingShader = context.GetStageShader("DeferredLightingStage");
  if (!lightingShader) {
    m_Logger->error(
        "DeferredLightingStage: No lightingShader found in context for stage 'DeferredLightingStage'");
    return;
  }
  if (lightingShader->GetProgramId() == 0) {
    m_Logger->error("DeferredLightingStage: lightingShader from context is not properly linked");
    return;
  }

  // 验证输入
  ValidateInputs(context);

  // 获取视口尺寸并验证/调整光照Framebuffer
  auto viewportSize = context.GetViewportSize();
  ValidateLightingFramebuffer(viewportSize);

  // 绑定光照Framebuffer 
  RenderCommand::Get().BindFrameBuffer(m_LightingFBO);

  // 清除输出目标
  RenderCommand::Get().Clear(
      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);

  // 设置光照渲染状态
  RenderCommand::Get().SetRenderState(m_LightingState);

  // 绑定着色器
  RenderCommand::Get().BindShader(lightingShader);

  // 绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetMainCameraInstance());

  // 绑定G-Buffer纹理
  BindGBufferTextures(context, lightingShader);

  // 绑定光源SSBO数据
  BindLightSSBOData(context, lightingShader);

  // 绑定阴影数据
  if (m_EnableShadows) {
    BindShadowMapTextures(context);
  }


  // 渲染全屏四边形
  RenderCommand::Get().DrawFullScreenQuad();

  // 解绑资源
  RenderCommand::Get().UnbindShader(lightingShader);
  RenderCommand::Get().UnbindFrameBuffer();

  // 将光照输出纹理存储到上下文供后续阶段使用
  auto lightingTexture = GetLightingOutputTexture();
  if (lightingTexture && lightingTexture->IsValid()) {
    context.SetRenderTarget("DeferredLightingOutput", lightingTexture);
    // 发布纹理完成事件
    RenderCommand::Get().PublishEventRuntimeTextureFinished(lightingTexture, "DeferredLighting");
    //m_Logger->trace("Stored deferred lighting output to context");
  }

  //m_Logger->trace("Deferred lighting pass completed");
}

void DeferredLightingStage::Shutdown()
{
  // 清理Framebuffer
  if (m_LightingFBO) {
    m_LightingFBO.reset();
  }

  m_Initialized = false;
  m_Logger->info("DeferredLightingStage shutdown completed");
}

void DeferredLightingStage::CreateLightingFramebuffer()
{
  // 配置Framebuffer规格
  FrameBufferSpec spec;
  spec.samples = 1;

  // 颜色附件配置 - 使用HDR格式存储光照结果
  FrameBufferAttachmentSpec colorSpec;
  colorSpec.type = RuntimeTextureType::Lighting_Combined;  // 仅考虑全部着色情况
  colorSpec.internalFormat = TextureFormat::RGBA16F;       // HDR输出
  colorSpec.generateMipmaps = false;

  spec.attachments = {colorSpec};  // 单个颜色附件

  // 创建Framebuffer
  m_LightingFBO = std::make_shared<FrameBuffer>(spec);

  if (!m_LightingFBO->IsComplete()) {
    m_Logger->error("Failed to create complete lighting framebuffer");
    return;
  }

  m_Logger->info("Created lighting framebuffer");
}

RuntimeTexturePtr DeferredLightingStage::GetLightingOutputTexture() const
{
  if (m_LightingFBO && !m_LightingFBO->GetColorAttachments().empty()) {
    return m_LightingFBO->GetColorAttachment(0);
  }
  return nullptr;
}

void DeferredLightingStage::SetupLightingRenderState()
{
  m_LightingState = std::make_shared<OpenGLRenderState>();

  // 延迟光照阶段：不需要深度测试和写入，需要混合（多光源叠加）
  m_LightingState->depthTest = false;
  m_LightingState->depthWrite = false;
  m_LightingState->blend = true;
  m_LightingState->cullFace = false;  // 全屏四边形不需要背面剔除
  m_LightingState->colorWriteR = true;
  m_LightingState->colorWriteG = true;
  m_LightingState->colorWriteB = true;
  m_LightingState->colorWriteA = true;

  // 需要确保不同光源的贡献正确叠加
  std::static_pointer_cast<OpenGLRenderState>(m_LightingState)->blendSrc = GL_ONE;
  std::static_pointer_cast<OpenGLRenderState>(m_LightingState)->blendDst = GL_ONE;
}



void DeferredLightingStage::BindGBufferTextures(RenderContext &context,
                                                std::shared_ptr<OpenGLShader> lightingShader)
{
  // 绑定所有G-Buffer纹理到对应的纹理单元
  for (const auto &type : GBuffer::GetTextureTypes()) {
    auto texture = context.GetGBufferTexture(type);
    if (texture && texture->IsValid()) {
      // 发布绑定命令
      RenderCommand::Get().BindRuntimeTexture(
          type, texture->GetHandle(), TextureTarget::TEXTURE_2D);

      //m_Logger->trace("Bound G-Buffer texture: {} to unit {}",
      //                GBuffer::GetTextureTypeName(type),
      //                static_cast<int>(type));
    }
    else {
      m_Logger->warn("Missing G-Buffer texture: {}", GBuffer::GetTextureTypeName(type));
    }
  }
}

void DeferredLightingStage::BindLightSSBOData(RenderContext &context,
                                              std::shared_ptr<OpenGLShader> lightingShader)
{
  // 从上下文获取LightManager
  auto lightManager = context.GetLightManager();

  if (lightManager.IsInitialized()) {

    // 绑定光源SSBO到着色器
    RenderCommand::Get().BindLightSSBO(lightManager.GetLightSSBO());

    m_Logger->trace("Bound light SSBO with lights");
  }
  else {
    m_Logger->warn("No LightManager available");
  }
}

void DeferredLightingStage::BindShadowMapTextures(RenderContext &context)
{
  uint32_t shadowTextureUnit = m_NextShadowTextureUnit;
  uint32_t boundShadowCount = 0;

  // 从上下文获取阴影贴图并绑定
  for (uint32_t i = 0; i < MAX_SHADOW_MAPS && shadowTextureUnit < 32; ++i) {
    auto shadowTexture = context.GetShadowMapTexture(0, i);
    if (shadowTexture && shadowTexture->IsValid()) {
      RenderCommand::Get().BindRuntimeTexture(
          shadowTexture->GetType(), shadowTexture->GetHandle(), TextureTarget::TEXTURE_2D);

      boundShadowCount++;
      shadowTextureUnit++;
    }
  }

  if (boundShadowCount > 0) {
    m_Logger->debug("Bound {} shadow maps", boundShadowCount);
  }
}


void DeferredLightingStage::ValidateInputs(RenderContext &context) const
{
  // 验证必要的G-Buffer纹理
  auto essentialTypes = {RuntimeTextureType::GBuffer_WorldPosDepth,
                         RuntimeTextureType::GBuffer_NormalScale,
                         RuntimeTextureType::GBuffer_BaseColorMatType};

  for (const auto &type : essentialTypes) {
    auto texture = context.GetGBufferTexture(type);
    if (!texture || !texture->IsValid()) {
      m_Logger->error("Missing essential G-Buffer texture: {}", GBuffer::GetTextureTypeName(type));
      throw std::runtime_error("Deferred lighting stage missing essential G-Buffer textures");
    }
  }
}

void DeferredLightingStage::ValidateLightingFramebuffer(const glm::vec2 &viewportSize)
{
  if (!m_LightingFBO) {
    m_Logger->error("Invalid lighting framebuffer");
    return;
  }

  auto currentSize = m_LightingFBO->GetSize();
  if (currentSize != viewportSize) {
    m_Logger->info("Resizing lighting FBO from {}x{} to {}x{}",
                   currentSize.x,
                   currentSize.y,
                   viewportSize.x,
                   viewportSize.y);
    m_LightingFBO->Resize(static_cast<uint32_t>(glm::max(viewportSize.x, 0.0f)),
                          static_cast<uint32_t>(glm::max(viewportSize.y, 0.0f)));

    if (!m_LightingFBO->IsComplete()) {
      m_Logger->error("Failed to resize lighting FBO");
      throw std::runtime_error("Lighting FBO resize failed");
    }
  }
}

}  // namespace mite
