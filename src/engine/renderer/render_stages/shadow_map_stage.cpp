#include "shadow_map_stage.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_cache.h"
#include "basic_shader/uniform_buffer.h"
#include "render_opengl/opengl_command.h"

namespace mite {
ShadowMapStage::ShadowMapStage() : RenderStage("ShadowMapStage")
{
  SetupShadowRenderState();
  m_Logger->info("ShadowMapStage created");
}

ShadowMapStage::~ShadowMapStage()
{
  m_Logger->info("ShadowMapStage destroyed");
}

void ShadowMapStage::SetShadowQuality(ShadowQuality quality)
{
  if (m_ShadowQuality != quality) {
    m_ShadowQuality = quality;

    // 重新创建阴影贴图
    if (m_Initialized) {
      Shutdown();
      Initialize();
    }

    m_Logger->info("Shadow quality set to {}", static_cast<int>(quality));
  }
}
void ShadowMapStage::SetShadowFilter(ShadowFilter filter)
{
  if (m_ShadowFilter != filter) {
    m_ShadowFilter = filter;
    m_Logger->info("Shadow filter set to {}", static_cast<int>(filter));
  }
}
void ShadowMapStage::SetShadowBias(float bias, float normalBias)
{
  m_ShadowBias = bias;
  m_NormalBias = normalBias;
  m_Logger->info("Shadow bias set to {}, normal bias: {}", bias, normalBias);
}

void ShadowMapStage::Initialize()
{
  if (m_Initialized) {
    m_Logger->warn("ShadowMapStage already initialized");
    return;
  }

  // 创建光源的阴影贴图
  CreateDirectionalShadowMap();
  CreatePointShadowMap();
  CreateSpotShadowMap();

  m_Initialized = true;
  m_Logger->info("ShadowMapStage initialization completed");
}

void ShadowMapStage::Execute(RenderContext &context)
{
  if (!m_Initialized) {
    m_Logger->warn("ShadowMapStage executed but not properly initialized");
    return;
  }

  if (!context.IsValid()) {
    m_Logger->warn("ShadowMapStage executed with invalid context");
    return;
  }

  // 从上下文获取ShadowMap着色器
  auto shadowShader = context.GetStageShader("ShadowMapStage");
  if (!shadowShader) {
    m_Logger->error("ShadowMapStage: No shadow shader found in context");
    return;
  }

  // 验证输入
  ValidateShadowInputs(context);

  // 设置阴影渲染状态
  RenderCommand::Get().SetRenderState(m_ShadowRenderState);

  // 绑定阴影着色器
  RenderCommand::Get().BindShader(shadowShader);

  // 按照类型获取光源
  LightManager &lightManager = context.GetLightManager();
  std::vector<std::shared_ptr<Light>> directionalLights = lightManager.GetLightsByType(
      LightType::DIRECTIONAL);
  std::vector<std::shared_ptr<Light>> pointLights = lightManager.GetLightsByType(LightType::POINT);
  std::vector<std::shared_ptr<Light>> spotLights = lightManager.GetLightsByType(LightType::SPOT);

  // 渲染光源的阴影贴图
  RenderDirectionalShadowMap(context, directionalLights);
  RenderPointShadowMap(context, pointLights);
  RenderSpotShadowMap(context, spotLights);

  // 解绑着色器
  RenderCommand::Get().UnbindShader(shadowShader);

  // 存储阴影贴图到上下文
  StoreShadowMapsToContext(context);

  m_Logger->trace("ShadowMap stage completed");
}

void ShadowMapStage::Shutdown()
{
  // 清理阴影贴图Framebuffer
  m_DirectionalShadowFBO = nullptr;
  m_PointShadowFBO = nullptr;
  m_SpotShadowFBO = nullptr;

  m_Initialized = false;
  m_Logger->info("ShadowMapStage shutdown completed");
}

RuntimeTexturePtr ShadowMapStage::GetDirectionalShadowMap() const
{
  if (!m_DirectionalShadowFBO) {
    m_Logger->warn("Directional shadow FBO is null");
    return nullptr;
  }
  // 返回整个2D数组纹理，着色器通过层索引访问特定光源和级联
  auto depthTexture = m_DirectionalShadowFBO->GetDepthAttachment();
  if (!depthTexture || !depthTexture->IsValid()) {
    m_Logger->warn("Directional shadow FBO has invalid depth attachment");
    return nullptr;
  }
  return depthTexture;
}
RuntimeTexturePtr ShadowMapStage::GetPointShadowMap() const
{
  if (!m_PointShadowFBO) {
    m_Logger->warn("Point shadow FBO is null");
    return nullptr;
  }
  // 返回整个立方体贴图数组，着色器通过层索引访问特定点光源
  auto depthTexture = m_PointShadowFBO->GetDepthAttachment();
  if (!depthTexture || !depthTexture->IsValid()) {
    m_Logger->warn("Point shadow FBO has invalid depth attachment");
    return nullptr;
  }
  // 验证是否为立方体贴图数组
  if (depthTexture->GetTarget() != TextureTarget::TEXTURE_CUBE_MAP_ARRAY) {
    m_Logger->warn("Point shadow texture is not a cube map array");
    return nullptr;
  }
  return depthTexture;
}
RuntimeTexturePtr ShadowMapStage::GetSpotShadowMap() const
{
  if (!m_SpotShadowFBO) {
    m_Logger->warn("Spot shadow FBO is null");
    return nullptr;
  }
  // 返回整个2D数组纹理，着色器通过层索引访问特定聚光灯
  auto depthTexture = m_SpotShadowFBO->GetDepthAttachment();
  if (!depthTexture || !depthTexture->IsValid()) {
    m_Logger->warn("Spot shadow FBO has invalid depth attachment");
    return nullptr;
  }
  return depthTexture;
}

void ShadowMapStage::CreateDirectionalShadowMap()
{
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 2D数组纹理存储所有方向光源的所有级联
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::ShadowMap_Directional;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_2D_ARRAY;     // 2D数组纹理
  depthSpec.arrayLayers = MAX_DIRECTIONAL_LIGHTS * MAX_CASCADES;  // 所有光源×所有级联
  spec.attachments.push_back(depthSpec);
  m_DirectionalShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_DirectionalShadowFBO->IsComplete()) {
    m_Logger->debug("Created directional shadow FBO with {} layers ({} lights × {} cascades)",
                    MAX_DIRECTIONAL_LIGHTS * MAX_CASCADES,
                    MAX_DIRECTIONAL_LIGHTS,
                    MAX_CASCADES);
  }
  else {
    m_Logger->error("Failed to create directional shadow FBO");
    m_DirectionalShadowFBO = nullptr;
  }
}
void ShadowMapStage::CreatePointShadowMap()
{
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 立方体贴图数组存储所有点光源
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::ShadowMap_Point;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_CUBE_MAP_ARRAY;
  depthSpec.arrayLayers = MAX_POINT_LIGHTS;
  spec.attachments.push_back(depthSpec);
  m_PointShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_PointShadowFBO->IsComplete()) {
    m_Logger->debug("Created point shadow FBO with {} cube maps", MAX_POINT_LIGHTS);
  }
  else {
    m_Logger->error("Failed to create point shadow FBO");
    m_PointShadowFBO = nullptr;
  }
}
void ShadowMapStage::CreateSpotShadowMap()
{
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 2D数组纹理存储所有聚光灯
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::ShadowMap_Spot;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_2D_ARRAY;
  depthSpec.arrayLayers = MAX_SPOT_LIGHTS;
  spec.attachments.push_back(depthSpec);
  m_SpotShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_SpotShadowFBO->IsComplete()) {
    m_Logger->debug("Created spot shadow FBO with {} layers", MAX_SPOT_LIGHTS);
  }
  else {
    m_Logger->error("Failed to create spot shadow FBO");
    m_SpotShadowFBO = nullptr;
  }
}

void ShadowMapStage::RenderDirectionalShadowMap(
    RenderContext &context, const std::vector<std::shared_ptr<Light>> &directionalLights)
{
  if (!m_DirectionalShadowFBO) {
    m_Logger->warn("Directional shadow FBO not available");
    return;
  }
  auto cameraInstance = context.GetMainCameraInstance();
  if (!cameraInstance) {
    m_Logger->warn("No main camera available for directional shadow rendering");
    return;
  }

  // 绑定方向光源阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_DirectionalShadowFBO);
  RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

  for (uint32_t lightIdx = 0;
       lightIdx < directionalLights.size() && lightIdx < MAX_DIRECTIONAL_LIGHTS;
       lightIdx++)
  {
    auto light = directionalLights[lightIdx];

    // 获取阴影数据
    ShadowMapData shadowData = light->PrepareShadowData(
        Transform(), cameraInstance->GetCameraTransform(), cameraInstance->GetProjectionMatrix());

    if (!shadowData.enabled || !shadowData.isValid) {
      continue;
    }
    // 为每个级联渲染
    for (uint32_t cascadeIdx = 0; cascadeIdx < shadowData.specific.directional.cascadeCount;
         cascadeIdx++)
    {
      // 使用分层渲染到数组纹理的特定层
      uint32_t layer = lightIdx * MAX_CASCADES + cascadeIdx;
      RenderCommand::Get().BindFrameBufferDepthLayer(m_DirectionalShadowFBO, layer);
      // 绑定阴影渲染上下文
      BindShadowRenderContext(lightIdx, cascadeIdx, 0, 0);
      // 渲染场景到当前级联
      RenderSceneToShadowMap(context,
                             context.GetRenderQueue()->GetItems(RenderQueue::QueueType::Opaque));
    }
  }
}
void ShadowMapStage::RenderPointShadowMap(RenderContext &context,
                                          const std::vector<std::shared_ptr<Light>> &pointLights)
{
  if (!m_PointShadowFBO) {
    m_Logger->warn("Point shadow FBO not available");
    return;
  }
  // 绑定点光源阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_PointShadowFBO);
  RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);
  for (uint32_t lightIdx = 0; lightIdx < pointLights.size() && lightIdx < MAX_POINT_LIGHTS;
       lightIdx++)
  {
    auto light = pointLights[lightIdx];

    // 获取阴影数据
    ShadowMapData shadowData = light->PrepareShadowData(
        light->GetTransform(), Transform(), glm::mat4(1.0f));

    if (!shadowData.enabled || !shadowData.isValid) {
      continue;
    }
    // 为立方体贴图数组的6个面分别渲染
    for (uint32_t faceIdx = 0; faceIdx < 6; faceIdx++) {
      RenderCommand::Get().BindFramebufferDepthCubeFace(m_SpotShadowFBO, lightIdx, faceIdx);
      BindShadowRenderContext(lightIdx, 0, faceIdx, 1);
      RenderSceneToShadowMap(context,
                             context.GetRenderQueue()->GetItems(RenderQueue::QueueType::Opaque));
    }
  }
}
void ShadowMapStage::RenderSpotShadowMap(RenderContext &context,
                                         const std::vector<std::shared_ptr<Light>> &spotLights)
{
  if (!m_SpotShadowFBO) {
    m_Logger->warn("Spot shadow FBO not available");
    return;
  }

  // 绑定聚光灯阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_SpotShadowFBO);
  RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);
  for (uint32_t lightIdx = 0; lightIdx < spotLights.size() && lightIdx < MAX_SPOT_LIGHTS;
       lightIdx++)
  {
    auto light = spotLights[lightIdx];

    // 获取阴影数据
    ShadowMapData shadowData = light->PrepareShadowData(
        light->GetTransform(), Transform(), glm::mat4(1.0f));

    if (!shadowData.enabled || !shadowData.isValid) {
      continue;
    }
    // 使用分层渲染到数组纹理的特定层
    RenderCommand::Get().BindFrameBufferDepthLayer(m_SpotShadowFBO, lightIdx);

    // 绑定阴影渲染上下文
    BindShadowRenderContext(lightIdx, 0, 0, 2);  // 类型2=聚光灯
    // 渲染场景几何体
    RenderSceneToShadowMap(context,
                           context.GetRenderQueue()->GetItems(RenderQueue::QueueType::Opaque));
  }
  m_Logger->debug("Rendered spot shadow maps for {} lights", spotLights.size());
}

void ShadowMapStage::SetupShadowRenderState()
{
  m_ShadowRenderState = std::make_shared<OpenGLRenderState>();

  // 阴影渲染需要深度测试和写入，不需要颜色输出和混合
  m_ShadowRenderState->depthTest = true;
  m_ShadowRenderState->depthWrite = true;
  m_ShadowRenderState->blend = false;
  m_ShadowRenderState->cullFace = true;
  m_ShadowRenderState->colorWriteR = false;  // 阴影贴图通常只写深度
  m_ShadowRenderState->colorWriteG = false;
  m_ShadowRenderState->colorWriteB = false;
  m_ShadowRenderState->colorWriteA = false;

  // 使用正面剔除减少阴影痤疮
  std::static_pointer_cast<OpenGLRenderState>(m_ShadowRenderState)->cullFaceMode = GL_FRONT;
}

void ShadowMapStage::BindShadowRenderContext(uint32_t lightIndex,
                                             uint32_t cascadeIndex,
                                             uint32_t faceIndex,
                                             uint32_t shadowMapType)
{
  // TODO: 实现ShadowRenderContextUBO的绑定
  // 需要更新u_ShadowContext UBO数据
  // ShadowRenderContextUniformBuffer shadowContext;
  // shadowContext.shadowRenderContext = glm::ivec4(lightIndex, cascadeIndex, faceIndex,
  // shadowMapType); shadowContext.shadowRenderParams = glm::vec4(0.0f, m_ShadowMapSize, 0.0f,
  // 0.0f); RenderCommand::Get().UpdateUBO(UBOResourceType::ShadowRenderContextUBO,
  // &shadowContext);

  m_Logger->trace("Bound shadow render context: light={}, cascade={}, face={}, type={}",
                  lightIndex,
                  cascadeIndex,
                  faceIndex,
                  shadowMapType);
}

void ShadowMapStage::RenderSceneToShadowMap(RenderContext &context,
                                            const std::vector<RenderableItem> &items)
{
  for (const auto &item : items) {
    if (!ValidateShadowRenderableItem(item)) {
      continue;
    }

    // 绑定材质UBO（用于Alpha测试）
    RenderCommand::Get().BindMaterialUBO(item.material);

    // 提交绘制调用
    RenderCommand::Get().SubmitDrawCall(item.mesh, context.GetStageShader("ShadowMapStage"));
  }
}

void ShadowMapStage::StoreShadowMapsToContext(RenderContext &context)
{
  // ==================== 修改：使用ShadowMapType枚举存储 ====================
  // 存储方向光源阴影贴图（2D数组纹理）
  if (m_DirectionalShadowFBO) {
    auto texture = m_DirectionalShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::DIRECTIONAL, texture);
      m_Logger->trace("Stored directional shadow map array");
    }
  }
  // 存储点光源阴影贴图（立方体贴图数组）
  if (m_PointShadowFBO) {
    auto texture = m_PointShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::POINT, texture);
      m_Logger->trace("Stored point shadow cube map array");
    }
  }
  // 存储聚光灯阴影贴图（2D数组纹理）
  if (m_SpotShadowFBO) {
    auto texture = m_SpotShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::SPOT, texture);
      m_Logger->trace("Stored spot shadow map array");
    }
  }
}

bool ShadowMapStage::ValidateShadowRenderableItem(const RenderableItem &item) const
{
  // 验证基础有效性
  if (!item.material || !item.mesh) {
    m_Logger->trace("ShadowMapStage: Invalid renderable item");
    return false;
  }

  // 验证网格有效性
  if (item.mesh->GetMesh()->GetModelHandle().vertexArray == 0) {
    m_Logger->trace("ShadowMapStage: Invalid mesh in renderable item");
    return false;
  }

  return true;
}

void ShadowMapStage::ValidateShadowInputs(RenderContext &context) const
{
  // 验证必要的输入数据
  auto renderQueue = context.GetRenderQueue();
  if (!renderQueue) {
    m_Logger->error("ShadowMapStage: No render queue available");
    throw std::runtime_error("ShadowMap stage missing render queue");
  }

  auto lightManager = context.GetLightManager();
  if (!lightManager.IsInitialized()) {
    m_Logger->error("ShadowMapStage: LightManager not initialized");
    throw std::runtime_error("ShadowMap stage missing light manager");
  }

  // 验证阴影着色器
  auto shadowShader = context.GetStageShader("ShadowMapStage");
  if (!shadowShader || shadowShader->GetProgramId() == 0) {
    m_Logger->error("ShadowMapStage: Invalid shadow shader");
    throw std::runtime_error("ShadowMap stage missing valid shadow shader");
  }
}
}  // namespace mite