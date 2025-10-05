#include "deferred_lighting_stage.h"
#include "basic_shader/gbuffer.h"
#include "basic_shader/shader_cache.h"
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

  // 加载延迟光照着色器
  m_LightingShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/lighting/deferred_lighting.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/lighting/deferred_lighting.frag.glsl").string());

  if (!m_LightingShader) {
    m_Logger->error("Failed to load deferred lighting shader");
    return;
  }

  // 创建全屏四边形
  CreateScreenQuad();

  // 创建默认尺寸的光照Framebuffer
  CreateLightingFramebuffer();

  m_Initialized = true;
  m_Logger->info("DeferredLightingStage initialization completed");
}

void DeferredLightingStage::Execute(RenderContext &context)
{
  if (!m_Initialized || !m_LightingShader) {
    m_Logger->warn("DeferredLightingStage executed but not properly initialized");
    return;
  }

  if (!context.IsValid()) {
    m_Logger->warn("DeferredLightingStage executed with invalid context");
    return;
  }

  // 验证输入
  ValidateInputs(context);

  // 获取视口尺寸并验证/调整光照Framebuffer
  auto viewportSize = context.GetViewportSize();
  ValidateLightingFramebuffer(viewportSize);

  // 绑定光照Framebuffer
  m_LightingFBO->Bind();

  // 清除输出目标
  RenderCommand::Get().Clear(
      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);

  // 设置光照渲染状态
  RenderCommand::Get().SetRenderState(m_LightingState);

  // 绑定着色器
  RenderCommand::Get().BindShader(m_LightingShader);

  // 绑定G-Buffer纹理
  BindGBufferTextures(context);

  // 绑定光源SSBO数据
  BindLightSSBOData(context);

  // 绑定阴影数据
  if (m_EnableShadows) {
    BindShadowData(context);
  }

  // 绑定相机和场景数据
  BindCameraAndSceneData(context);

  // 渲染全屏四边形
  RenderFullScreenQuad();

  // 解绑资源
  RenderCommand::Get().UnbindShader(m_LightingShader);
  m_LightingFBO->Unbind();

  // 将光照输出纹理存储到上下文供后续阶段使用
  auto lightingTexture = GetLightingOutputTexture();
  if (lightingTexture && lightingTexture->isValid()) {
    context.SetRenderTarget("DeferredLightingOutput", lightingTexture);
    // 发布纹理完成事件
    RenderCommand::Get().PublishEventRuntimeTextureFinished(lightingTexture, "DeferredLighting");
    m_Logger->trace("Stored deferred lighting output to context");
  }

  m_Logger->trace("Deferred lighting pass completed");
}

void DeferredLightingStage::Shutdown()
{
  // 清理Framebuffer
  if (m_LightingFBO) {
    m_LightingFBO.reset();
  }

  // 清理全屏四边形
  if (m_ScreenQuadVAO != 0) {
    glDeleteVertexArrays(1, &m_ScreenQuadVAO);
    m_ScreenQuadVAO = 0;
  }
  if (m_ScreenQuadVBO != 0) {
    glDeleteBuffers(1, &m_ScreenQuadVBO);
    m_ScreenQuadVBO = 0;
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
  colorSpec.type = RuntimeTextureType::Lighting_Combined;  // 明确指定用途
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

void DeferredLightingStage::CreateScreenQuad()
{
  // 全屏四边形顶点数据 (位置, UV)
  float quadVertices[] = {
    // 位置          // UV
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
  };

  glGenVertexArrays(1, &m_ScreenQuadVAO);
  glGenBuffers(1, &m_ScreenQuadVBO);

  glBindVertexArray(m_ScreenQuadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

  // 位置属性
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // UV属性
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  glBindVertexArray(0);
}

void DeferredLightingStage::BindGBufferTextures(RenderContext &context)
{
  // 绑定所有G-Buffer纹理到对应的纹理单元
  for (const auto &type : GBuffer::GetTextureTypes()) {
    auto texture = context.GetGBufferTexture(type);
    if (texture && texture->isValid()) {
      // 根据纹理类型设置到对应的uniform
      std::string uniformName = GBuffer::GetTextureTypeName(type);
      m_LightingShader->SetInt(uniformName, static_cast<int>(type));
      RenderCommand::Get().BindTexture(texture->getHandle(), static_cast<uint32_t>(type));

      m_Logger->trace("Bound G-Buffer texture: {} to unit {}",
                      GBuffer::GetTextureTypeName(type),
                      static_cast<int>(type));
    }
    else {
      m_Logger->warn("Missing G-Buffer texture: {}", GBuffer::GetTextureTypeName(type));
    }
  }
}

void DeferredLightingStage::BindLightSSBOData(RenderContext &context)
{
  // 优先使用外部设置的LightManager
  auto lightManager = m_LightManager;
  if (!lightManager) {
    // 尝试从上下文获取LightManager
    lightManager = context.GetTemporaryResource<LightManager>("LightManager");
  }

  if (lightManager && lightManager->IsInitialized()) {
    // 绑定光源SSBO到着色器
    lightManager->SetupShaderBinding(m_LightingShader);
    lightManager->BindLightSSBO();

    // 设置光源统计信息到uniform
    size_t enabledLightCount = lightManager->GetEnabledLightCount();
    m_LightingShader->SetInt("u_EnabledLightCount", static_cast<int>(enabledLightCount));

    m_Logger->trace("Bound light SSBO with {} enabled lights", enabledLightCount);
  }
  else {
    // 没有LightManager时的后备方案
    m_Logger->warn("No LightManager available, using default lighting");
    m_LightingShader->SetInt("u_EnabledLightCount", 1);
  }
}

void DeferredLightingStage::BindShadowData(RenderContext &context)
{
  BindShadowMapTextures(context);
  SetupShadowUniforms(m_LightingShader);
}

void DeferredLightingStage::BindShadowMapTextures(RenderContext &context)
{
  uint32_t shadowTextureUnit = m_NextShadowTextureUnit;
  uint32_t boundShadowCount = 0;

  // 从上下文获取阴影贴图并绑定
  for (uint32_t i = 0; i < MAX_SHADOW_MAPS && shadowTextureUnit < 32; ++i) {
    auto shadowTexture = context.GetShadowMapTexture(0, i);
    if (shadowTexture && shadowTexture->isValid()) {
      RenderCommand::Get().BindTexture(shadowTexture->getHandle(), shadowTextureUnit);

      std::string uniformName = "u_ShadowMaps[" + std::to_string(boundShadowCount) + "]";
      m_LightingShader->SetInt(uniformName, shadowTextureUnit);

      boundShadowCount++;
      shadowTextureUnit++;
    }
  }

  m_LightingShader->SetInt("u_ShadowMapCount", static_cast<int>(boundShadowCount));

  if (boundShadowCount > 0) {
    m_Logger->debug("Bound {} shadow maps", boundShadowCount);
  }
}

void DeferredLightingStage::SetupShadowUniforms(std::shared_ptr<OpenGLShader> shader)
{
  shader->SetFloat("u_ShadowBias", 0.005f);
  shader->SetFloat("u_ShadowNormalBias", 0.02f);
  shader->SetFloat("u_ShadowPCFRadius", 2.0f);
  shader->SetInt("u_EnableCSM", 1);
  shader->SetFloat("u_CascadeSplitLambda", 0.95f);
}

void DeferredLightingStage::BindCameraAndSceneData(RenderContext &context)
{
  // 绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetCameraInstance());

  // 设置视口尺寸
  auto viewportSize = context.GetViewportSize();
  m_LightingShader->SetVec2("u_ViewportSize", glm::vec2(viewportSize.x, viewportSize.y));

  // 设置相机位置
  auto cameraPos = context.GetCameraInstance().GetCameraTransform().GetPosition();
  m_LightingShader->SetVec3("u_CameraPosition", cameraPos);

  // 设置时间
  static float time = 0.0f;
  time += 0.016f;
  m_LightingShader->SetFloat("u_Time", time);
}

void DeferredLightingStage::RenderFullScreenQuad()
{
  glBindVertexArray(m_ScreenQuadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void DeferredLightingStage::ValidateInputs(RenderContext &context) const
{
  // 验证必要的G-Buffer纹理
  auto essentialTypes = {RuntimeTextureType::GBuffer_WorldPosDepth,
                         RuntimeTextureType::GBuffer_NormalScale,
                         RuntimeTextureType::GBuffer_BaseColorMatType};

  for (const auto &type : essentialTypes) {
    auto texture = context.GetGBufferTexture(type);
    if (!texture || !texture->isValid()) {
      m_Logger->error("Missing essential G-Buffer texture: {}", GBuffer::GetTextureTypeName(type));
      throw std::runtime_error("Deferred lighting stage missing essential G-Buffer textures");
    }
  }
}

void DeferredLightingStage::ValidateLightingFramebuffer(const glm::uvec2 &viewportSize)
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
    m_LightingFBO->Resize(viewportSize.x, viewportSize.y);

    if (!m_LightingFBO->IsComplete()) {
      m_Logger->error("Failed to resize lighting FBO");
      throw std::runtime_error("Lighting FBO resize failed");
    }
  }
}

void DeferredLightingStage::SetLightingShader(std::shared_ptr<OpenGLShader> shader)
{
  m_LightingShader = shader;
  if (shader) {
    m_Logger->info("Lighting shader updated for DeferredLightingStage");
  }
}

void DeferredLightingStage::SetLightManager(std::shared_ptr<LightManager> lightManager)
{
  m_LightManager = lightManager;
  if (lightManager) {
    m_Logger->info("LightManager set for DeferredLightingStage");
  }
}

}  // namespace mite
