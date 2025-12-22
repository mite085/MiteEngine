#include "shadow_map_stage.h"

#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_cache.h"
#include "basic_shader/uniform_buffer.h"
#include "render_opengl/opengl_command.h"

namespace mite {
ShadowMapStage::ShadowMapStage() : RenderStage("ShadowMapStage") {
  m_Logger->info("ShadowMapStage created");
}

ShadowMapStage::~ShadowMapStage() {
  m_Logger->info("ShadowMapStage destroyed");
}

void ShadowMapStage::SetShadowQuality(ShadowQuality quality) {
  if (m_ShadowQuality != quality && m_Initialized) {
    // 清理已创建资源
    Shutdown();

    m_ShadowQuality = quality;

    // 重新创建光源的阴影贴图
    CreateDirectionalShadowMap();
    CreatePointShadowMap();
    CreateSpotShadowMap();

    // 创建对应的ShadowRenderContextUBO对象
    CreateShadowRenderContextUniformBuffer();

    m_Logger->info("Shadow quality set to {}", static_cast<int>(quality));
  }
}
void ShadowMapStage::SetShadowFilter(ShadowFilter filter) {
  if (m_ShadowFilter != filter) {
    m_ShadowFilter = filter;
    m_Logger->info("Shadow filter set to {}", static_cast<int>(filter));
  }
}
void ShadowMapStage::SetShadowBias(float bias, float normalBias) {
  m_ShadowBias = bias;
  m_NormalBias = normalBias;
  m_Logger->info("Shadow bias set to {}, normal bias: {}", bias, normalBias);
}

void ShadowMapStage::Initialize([[maybe_unused]] RenderContext &context) {
  if (m_Initialized) {
    m_Logger->warn("ShadowMapStage already initialized");
    return;
  }

  // 初始化Shadow渲染状态
  SetupShadowRenderState();

  // 创建光源的阴影贴图
  CreateDirectionalShadowMap();
  CreatePointShadowMap();
  CreateSpotShadowMap();

  // 创建对应的ShadowRenderContextUBO对象
  CreateShadowRenderContextUniformBuffer();

  m_Initialized = true;
  m_Logger->info("ShadowMapStage initialization completed");
}

void ShadowMapStage::Execute(RenderContext &context) {
  if (!m_Initialized) {
    m_Logger->warn("ShadowMapStage executed but not properly initialized");
    return;
  }

  if (!context.IsValid()) {
    m_Logger->warn("ShadowMapStage executed with invalid context");
    return;
  }

  // 验证输入
  ValidateShadowInputs(context);

  // 更新Shadow Map UBO
  LightManager &lightManager = context.GetLightManager();
  std::shared_ptr<CameraInstance> cameraInstance =
      context.GetMainCameraInstance();
  if (!cameraInstance) {
    m_Logger->warn("No main camera available for directional shadow rendering");
    return;
  }

  lightManager.UpdateLightShadowUBO(
      cameraInstance,
      glm::vec4(m_ShadowBias, m_NormalBias, m_ShadowFilter, m_ShadowQuality));

  // 设置阴影渲染状态
  RenderCommand::Get().SetRenderState(m_ShadowRenderState);

  // 绑定阴影UBO
  RenderCommand::Get().BindShadowUBO(lightManager.GetShadowInstance());

  // 绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetMainCameraInstance());

  // 按照类型获取光源
  std::vector<std::shared_ptr<Light>> directionalLights =
      lightManager.GetLightsByType(LightType::DIRECTIONAL);
  std::vector<std::shared_ptr<Light>> pointLights =
      lightManager.GetLightsByType(LightType::POINT);
  std::vector<std::shared_ptr<Light>> spotLights =
      lightManager.GetLightsByType(LightType::SPOT);

  // 渲染光源的阴影贴图
  RenderDirectionalShadowMap(context, directionalLights);
  RenderPointShadowMap(context, pointLights);
  RenderSpotShadowMap(context, spotLights);

  // 存储阴影贴图到上下文
  StoreShadowMapsToContext(context);

  // 调试专用：当前阶段提交完毕后直接执行。
  RenderCommand::Get().Flush();

  m_Logger->trace("ShadowMap stage completed");
}

void ShadowMapStage::Shutdown() {
  // 清理阴影贴图Framebuffer
  m_DirectionalShadowFBO = nullptr;
  m_PointShadowFBO = nullptr;
  m_SpotShadowFBO = nullptr;

  // 清理阴影上下文UBO
  m_DirectionallightUBOs.fill(nullptr);
  m_PointlightUBOs.fill(nullptr);
  m_SpotlightUBOs.fill(nullptr);

  // 标记为未初始化
  m_Initialized = false;
  m_Logger->info("ShadowMapStage shutdown completed");
}

RuntimeTexturePtr ShadowMapStage::GetDirectionalShadowMap() const {
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
RuntimeTexturePtr ShadowMapStage::GetPointShadowMap() const {
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
RuntimeTexturePtr ShadowMapStage::GetSpotShadowMap() const {
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

void ShadowMapStage::CreateDirectionalShadowMap() {
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 2D数组纹理存储所有方向光源的所有级联
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::Depth;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_2D_ARRAY;  // 2D数组纹理
  depthSpec.arrayLayers =
      MAX_DIRECTIONAL_LIGHTS * MAX_CASCADES;  // 所有光源×所有级联
  spec.attachments.push_back(depthSpec);
  m_DirectionalShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_DirectionalShadowFBO->IsComplete()) {
    m_Logger->debug(
        "Created directional shadow FBO with {} layers ({} lights with {} "
        "cascades)",
        MAX_DIRECTIONAL_LIGHTS * MAX_CASCADES, MAX_DIRECTIONAL_LIGHTS,
        MAX_CASCADES);
  } else {
    m_Logger->error("Failed to create directional shadow FBO");
    m_DirectionalShadowFBO = nullptr;
  }
}
void ShadowMapStage::CreatePointShadowMap() {
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 立方体贴图数组存储所有点光源
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::Depth;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_CUBE_MAP_ARRAY;
  depthSpec.arrayLayers = MAX_POINT_LIGHTS * 6;
  spec.attachments.push_back(depthSpec);
  m_PointShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_PointShadowFBO->IsComplete()) {
    m_Logger->debug("Created point shadow FBO with {} cube maps",
                    MAX_POINT_LIGHTS);
  } else {
    m_Logger->error("Failed to create point shadow FBO");
    m_PointShadowFBO = nullptr;
  }
}
void ShadowMapStage::CreateSpotShadowMap() {
  FrameBufferSpec spec;
  spec.samples = 1;
  spec.width = static_cast<uint32_t>(m_ShadowQuality);
  spec.height = static_cast<uint32_t>(m_ShadowQuality);

  // 深度附件配置 - 2D数组纹理存储所有聚光灯
  FrameBufferAttachmentSpec depthSpec;
  depthSpec.type = RuntimeTextureType::Depth;
  depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
  depthSpec.generateMipmaps = false;
  depthSpec.internalTarget = TextureTarget::TEXTURE_2D_ARRAY;
  depthSpec.arrayLayers = MAX_SPOT_LIGHTS;
  spec.attachments.push_back(depthSpec);
  m_SpotShadowFBO = std::make_shared<FrameBuffer>(spec);
  if (m_SpotShadowFBO->IsComplete()) {
    m_Logger->debug("Created spot shadow FBO with {} layers", MAX_SPOT_LIGHTS);
  } else {
    m_Logger->error("Failed to create spot shadow FBO");
    m_SpotShadowFBO = nullptr;
  }
}

void ShadowMapStage::CreateShadowRenderContextUniformBuffer() {
  // 默认填充清空
  m_DirectionallightUBOs.fill(nullptr);
  m_PointlightUBOs.fill(nullptr);
  m_SpotlightUBOs.fill(nullptr);

  // 创建UBO，执行初始化
  for (size_t i = 0; i < m_DirectionallightUBOs.size(); i++) {
    m_DirectionallightUBOs.at(i) = std::make_shared<ShaderUBO>(
        sizeof(ShadowRenderContextUniformBuffer),
        BindingPointManager::Get().GetShadowRenderContextUBOBinding(),
        GL_DYNAMIC_DRAW);
    m_DirectionallightUBOs.at(i)->Initialize();
  }
  for (size_t i = 0; i < m_PointlightUBOs.size(); i++) {
    m_PointlightUBOs.at(i) = std::make_shared<ShaderUBO>(
        sizeof(ShadowRenderContextUniformBuffer),
        BindingPointManager::Get().GetShadowRenderContextUBOBinding(),
        GL_DYNAMIC_DRAW);
    m_PointlightUBOs.at(i)->Initialize();
  }
  for (size_t i = 0; i < m_SpotlightUBOs.size(); i++) {
    m_SpotlightUBOs.at(i) = std::make_shared<ShaderUBO>(
        sizeof(ShadowRenderContextUniformBuffer),
        BindingPointManager::Get().GetShadowRenderContextUBOBinding(),
        GL_DYNAMIC_DRAW);
    m_SpotlightUBOs.at(i)->Initialize();
  }
}

void ShadowMapStage::RenderDirectionalShadowMap(
    RenderContext &context,
    const std::vector<std::shared_ptr<Light>> &directionalLights) {
  if (!m_DirectionalShadowFBO) {
    m_Logger->warn("Directional shadow FBO not available");
    return;
  }

  // 从上下文获取ShadowMap着色器
  auto shadowShader = context.GetStageShader("ShadowMapStage");
  if (!shadowShader) {
    m_Logger->error("ShadowMapStage: No shadow shader found in context");
    return;
  }

  // 绑定方向光源阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_DirectionalShadowFBO);

  for (uint32_t lightIdx = 0;
       lightIdx < directionalLights.size() && lightIdx < MAX_DIRECTIONAL_LIGHTS;
       lightIdx++) {
    std::shared_ptr<Light> light = directionalLights[lightIdx];

    // 为每个级联渲染
    for (uint32_t cascadeIdx = 0; cascadeIdx < MAX_CASCADES; cascadeIdx++) {
      // 使用分层渲染到数组纹理的特定层
      uint32_t layer = lightIdx * MAX_CASCADES + cascadeIdx;
      RenderCommand::Get().BindFrameBufferDepthLayer(m_DirectionalShadowFBO,
                                                     layer);
      RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

      // 绑定阴影着色器
      RenderCommand::Get().BindShader(shadowShader);

      // 绑定阴影渲染上下文
      BindShadowRenderContext(lightIdx, cascadeIdx, 0, 0);
      // 渲染场景到当前级联（仅包含不透明和Alpha测试，半透明物体需要更复杂的ShadowMap策略实现阴影）
      RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                          RenderableItemType::Opaque));
      RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                          RenderableItemType::AlphaTest));

      // 解绑着色器
      RenderCommand::Get().UnbindShader(shadowShader);
    }
  }
  RenderCommand::Get().UnbindFrameBuffer();
  m_Logger->trace("Rendered directional shadow maps for {} lights",
                  directionalLights.size());
}
void ShadowMapStage::RenderPointShadowMap(
    RenderContext &context,
    const std::vector<std::shared_ptr<Light>> &pointLights) {
  if (!m_PointShadowFBO) {
    m_Logger->warn("Point shadow FBO not available");
    return;
  }

  // 从上下文获取ShadowMap着色器
  auto shadowShader = context.GetStageShader("ShadowMapStage");
  if (!shadowShader) {
    m_Logger->error("ShadowMapStage: No shadow shader found in context");
    return;
  }

  // 绑定点光源阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_PointShadowFBO);

  for (uint32_t lightIdx = 0;
       lightIdx < pointLights.size() && lightIdx < MAX_POINT_LIGHTS;
       lightIdx++) {
    std::shared_ptr<Light> light = pointLights[lightIdx];

    // 为立方体贴图数组的6个面分别执行渲染
    for (uint32_t faceIdx = 0; faceIdx < 6; faceIdx++) {
      RenderCommand::Get().BindFramebufferDepthCubeFace(m_PointShadowFBO,
                                                        lightIdx, faceIdx);
      RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

      // 绑定阴影着色器
      RenderCommand::Get().BindShader(shadowShader);

      BindShadowRenderContext(lightIdx, 0, faceIdx, 1);
      // 渲染场景到当前立方体贴图的面（仅包含不透明和Alpha测试，半透明物体需要更复杂的ShadowMap策略实现阴影）
      RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                          RenderableItemType::Opaque));
      RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                          RenderableItemType::AlphaTest));

      // 解绑着色器
      RenderCommand::Get().UnbindShader(shadowShader);
    }
  }
  RenderCommand::Get().UnbindFrameBuffer();
  m_Logger->trace("Rendered point shadow maps for {} lights",
                  pointLights.size());
}
void ShadowMapStage::RenderSpotShadowMap(
    RenderContext &context,
    const std::vector<std::shared_ptr<Light>> &spotLights) {
  if (!m_SpotShadowFBO) {
    m_Logger->warn("Spot shadow FBO not available");
    return;
  }

  // 从上下文获取ShadowMap着色器
  auto shadowShader = context.GetStageShader("ShadowMapStage");
  if (!shadowShader) {
    m_Logger->error("ShadowMapStage: No shadow shader found in context");
    return;
  }

  // 绑定聚光灯阴影FBO
  RenderCommand::Get().BindFrameBuffer(m_SpotShadowFBO);

  for (uint32_t lightIdx = 0;
       lightIdx < spotLights.size() && lightIdx < MAX_SPOT_LIGHTS; lightIdx++) {
    std::shared_ptr<Light> light = spotLights[lightIdx];

    // 使用分层渲染到数组纹理的特定层
    RenderCommand::Get().BindFrameBufferDepthLayer(m_SpotShadowFBO, lightIdx);
    RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

    // 绑定阴影着色器
    RenderCommand::Get().BindShader(shadowShader);

    // 绑定阴影渲染上下文
    BindShadowRenderContext(lightIdx, 0, 0, 2);  // 类型2=聚光灯
    // 渲染场景几何体（仅包含不透明和Alpha测试，半透明物体需要更复杂的ShadowMap策略实现阴影）
    RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                        RenderableItemType::Opaque));
    RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(
                                        RenderableItemType::AlphaTest));

    // 解绑着色器
    RenderCommand::Get().UnbindShader(shadowShader);
  }
  RenderCommand::Get().UnbindFrameBuffer();
  m_Logger->trace("Rendered spot shadow maps for {} lights", spotLights.size());
}

void ShadowMapStage::SetupShadowRenderState() {
  m_ShadowRenderState = std::make_shared<OpenGLRenderState>();

  // 阴影渲染需要深度测试和写入，不需要颜色输出和混合
  // （TODO：OpenGL管线的不同stage之间出现了状态污染）
  // （先设定的和后设定的状态会互相影响）
  // （且暂未找出解决方案）
  m_ShadowRenderState->depthTest = true;
  m_ShadowRenderState->depthWrite = true;
  m_ShadowRenderState->blend = false;
  m_ShadowRenderState->cullFace = true;
  m_ShadowRenderState->colorWriteR = true;
  m_ShadowRenderState->colorWriteG = true;
  m_ShadowRenderState->colorWriteB = true;
  m_ShadowRenderState->colorWriteA = true;

  // 使用正面剔除减少阴影痤疮
  std::static_pointer_cast<OpenGLRenderState>(m_ShadowRenderState)
      ->cullFaceMode = GL_FRONT;
}

void ShadowMapStage::BindShadowRenderContext(uint32_t lightIndex,
                                             uint32_t cascadeIndex,
                                             uint32_t faceIndex,
                                             uint32_t shadowMapType) {
  // 创建UBO数据
  ShadowRenderContextUniformBuffer shadowContext;
  shadowContext.shadowRenderContext =
      glm::ivec4(lightIndex, cascadeIndex, faceIndex, shadowMapType);
  shadowContext.shadowRenderParams =
      glm::vec4(0.0f, static_cast<uint32_t>(m_ShadowQuality), 0.0f, 0.0f);

  // 获取UBO对象
  std::shared_ptr<ShaderUBO> ubo = nullptr;
  switch (shadowMapType) {
    case 0:
      ubo = m_DirectionallightUBOs.at(lightIndex * MAX_CASCADES + cascadeIndex);
      break;
    case 1:
      ubo = m_PointlightUBOs.at(lightIndex * 6 + faceIndex);
      break;
    case 2:
      ubo = m_SpotlightUBOs.at(lightIndex);
      break;
    default:
      m_Logger->critical("Invalid Shadow Map Type: {}", shadowMapType);
      return;
  }

  // 执行Update操作，更新UBO数据
  ubo->UpdateData(&shadowContext, sizeof(ShadowRenderContextUniformBuffer));

  // 实现ShadowRenderContextUBO的绑定
  RenderCommand::Get().BindShadowRenderContextUBO(ubo);

  m_Logger->trace(
      "Bound shadow render context: light={}, cascade={}, face={}, type={}",
      lightIndex, cascadeIndex, faceIndex, shadowMapType);
}

void ShadowMapStage::RenderSceneToShadowMap(
    [[maybe_unused]] RenderContext &context,
    const std::vector<RenderableItem> &items) {
  for (const auto &item : items) {
    if (!ValidateShadowRenderableItem(item)) {
      continue;
    }

    // 提交绘制调用
    RenderCommand::Get().BindMaterialUBO(item.material);
    RenderCommand::Get().SubmitDrawCall(item.mesh);
  }
}

void ShadowMapStage::StoreShadowMapsToContext(RenderContext &context) {
  // ==================== 使用ShadowMapType枚举存储 ====================
  //
  // 存储方向光源阴影贴图（2D数组纹理）
  if (m_DirectionalShadowFBO) {
    RuntimeTexturePtr texture = m_DirectionalShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::DIRECTIONAL, texture);
      m_Logger->trace("Stored directional shadow map array");
    }
  }
  // 存储点光源阴影贴图（立方体贴图数组）
  if (m_PointShadowFBO) {
    RuntimeTexturePtr texture = m_PointShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::POINT, texture);
      m_Logger->trace("Stored point shadow cube map array");
    }
  }
  // 存储聚光灯阴影贴图（2D数组纹理）
  if (m_SpotShadowFBO) {
    RuntimeTexturePtr texture = m_SpotShadowFBO->GetDepthAttachment();
    if (texture && texture->IsValid()) {
      context.SetShadowMapTexture(LightType::SPOT, texture);
      m_Logger->trace("Stored spot shadow map array");
    }
  }
}

bool ShadowMapStage::ValidateShadowRenderableItem(
    const RenderableItem &item) const {
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

void ShadowMapStage::ValidateShadowInputs(RenderContext &context) const {
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