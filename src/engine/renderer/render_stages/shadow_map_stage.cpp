#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_cache.h"
#include "basic_shader/uniform_buffer.h"
#include "render_opengl/opengl_command.h"
#include "shadow_map_stage.h"

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

void ShadowMapStage::Initialize()
{
  if (m_Initialized) {
    m_Logger->warn("ShadowMapStage already initialized");
    return;
  }

  // 创建各种光源的阴影贴图
  CreateDirectionalShadowMaps();
  CreatePointShadowMaps();
  CreateSpotShadowMaps();

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

  // 更新阴影矩阵（需要在C++端计算）
  UpdateShadowMatrices(context);

  // 设置阴影渲染状态
  RenderCommand::Get().SetRenderState(m_ShadowRenderState);

  // 绑定阴影着色器
  RenderCommand::Get().BindShader(shadowShader);

  // 渲染各种光源的阴影贴图
  RenderDirectionalShadowMaps(context);
  RenderPointShadowMaps(context);
  RenderSpotShadowMaps(context);

  // 解绑着色器
  RenderCommand::Get().UnbindShader(shadowShader);

  // 存储阴影贴图到上下文
  // TODO: 需要实现上下文中的阴影贴图存储接口
  m_Logger->trace("ShadowMap stage completed");
}

void ShadowMapStage::Shutdown()
{
  // 清理阴影贴图Framebuffer
  m_DirectionalShadowFBOs.clear();
  m_PointShadowFBOs.clear();
  m_SpotShadowFBOs.clear();

  m_Initialized = false;
  m_Logger->info("ShadowMapStage shutdown completed");
}

void ShadowMapStage::CreateDirectionalShadowMaps()
{
  m_DirectionalShadowFBOs.clear();

  // 为每个方向光源创建一个FBO，使用2D数组纹理存储所有级联
  for (uint32_t i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
    FrameBufferSpec spec;
    spec.samples = 1;
    spec.width = m_ShadowMapSize;
    spec.height = m_ShadowMapSize;

    // 深度附件配置 - 2D数组纹理存储所有级联
    FrameBufferAttachmentSpec depthSpec;
    depthSpec.type = RuntimeTextureType::ShadowMap_Directional;
    depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
    depthSpec.generateMipmaps = false;

    spec.attachments.push_back(depthSpec);

    auto fbo = std::make_shared<FrameBuffer>(spec);
    if (fbo->IsComplete()) {
      m_DirectionalShadowFBOs.push_back(fbo);
      m_Logger->debug(
          "Created directional shadow FBO for light {} with {} cascades", i, MAX_CASCADES);
    }
    else {
      m_Logger->error("Failed to create directional shadow FBO for light {}", i);
      m_DirectionalShadowFBOs.push_back(nullptr);
    }
  }
}
void ShadowMapStage::CreatePointShadowMaps()
{
  m_PointShadowFBOs.clear();

  // 为每个点光源创建一个FBO，使用立方体贴图
  for (uint32_t i = 0; i < MAX_POINT_LIGHTS; i++) {
    FrameBufferSpec spec;
    spec.samples = 1;
    spec.width = m_ShadowMapSize;
    spec.height = m_ShadowMapSize;

    // 深度附件配置 - 立方体贴图
    FrameBufferAttachmentSpec depthSpec;
    depthSpec.type = RuntimeTextureType::ShadowMap_Point;
    depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
    depthSpec.generateMipmaps = false;

    spec.attachments.push_back(depthSpec);

    auto fbo = std::make_shared<FrameBuffer>(spec);
    if (fbo->IsComplete()) {
      m_PointShadowFBOs.push_back(fbo);
      m_Logger->debug("Created point shadow FBO for light {}", i);
    }
    else {
      m_Logger->error("Failed to create point shadow FBO for light {}", i);
      m_PointShadowFBOs.push_back(nullptr);
    }
  }
}
void ShadowMapStage::CreateSpotShadowMaps()
{
  m_SpotShadowFBOs.clear();

  // 为每个聚光灯创建一个FBO，使用普通2D纹理
  for (uint32_t i = 0; i < MAX_SPOT_LIGHTS; i++) {
    FrameBufferSpec spec;
    spec.samples = 1;
    spec.width = m_ShadowMapSize;
    spec.height = m_ShadowMapSize;

    // 深度附件配置 - 普通2D纹理
    FrameBufferAttachmentSpec depthSpec;
    depthSpec.type = RuntimeTextureType::ShadowMap_Spot;
    depthSpec.internalFormat = TextureFormat::DEPTH_COMPONENT32;
    depthSpec.generateMipmaps = false;

    spec.attachments.push_back(depthSpec);

    auto fbo = std::make_shared<FrameBuffer>(spec);
    if (fbo->IsComplete()) {
      m_SpotShadowFBOs.push_back(fbo);
      m_Logger->debug("Created spot shadow FBO for light {}", i);
    }
    else {
      m_Logger->error("Failed to create spot shadow FBO for light {}", i);
      m_SpotShadowFBOs.push_back(nullptr);
    }
  }
}

void ShadowMapStage::RenderDirectionalShadowMaps(RenderContext &context)
{
  auto lightManager = context.GetLightManager();
  if (!lightManager.IsInitialized())
    return;
  // TODO: 获取实际的方向光源数量
  uint32_t directionalLightCount = 0; // lightManager.GetDirectionalLightCount();

  for (uint32_t lightIdx = 0; lightIdx < directionalLightCount; lightIdx++) {
    if (lightIdx >= m_DirectionalShadowFBOs.size() || !m_DirectionalShadowFBOs[lightIdx]) {
      continue;
    }

    // 绑定方向光源阴影FBO
    RenderCommand::Get().BindFrameBuffer(m_DirectionalShadowFBOs[lightIdx]);

    // 清除深度缓冲区
    RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

    // 为每个级联渲染
    for (uint32_t cascadeIdx = 0; cascadeIdx < MAX_CASCADES; cascadeIdx++) {
      // 设置视口到当前级联层
      // TODO: 需要FBO支持分层渲染的视口设置

      // 绑定阴影渲染上下文
      BindShadowRenderContext(lightIdx, cascadeIdx, 0, 0);

      // 渲染场景到当前级联
      RenderSceneToShadowMap(context, context.GetRenderQueue()->GetItems(RenderQueue::QueueType::Opaque));
    }
  }
}
void ShadowMapStage::RenderPointShadowMaps(RenderContext &context)
{
  auto lightManager = context.GetLightManager();
  if (!lightManager.IsInitialized())
    return;
  // TODO: 获取实际的点光源数量
  uint32_t pointLightCount = 0; // lightManager.GetDirectionalLightCount();

  for (uint32_t lightIdx = 0; lightIdx < pointLightCount; lightIdx++) {
    if (lightIdx >= m_PointShadowFBOs.size() || !m_PointShadowFBOs[lightIdx]) {
      continue;
    }

    // 绑定点光源阴影FBO（立方体贴图）
    RenderCommand::Get().BindFrameBuffer(m_PointShadowFBOs[lightIdx]);
    RenderCommand::Get().Clear(GL_DEPTH_BUFFER_BIT, glm::vec4(0.0f), 1.0f);

    // 为立方体贴图的6个面分别渲染
    for (uint32_t faceIdx = 0; faceIdx < 6; faceIdx++) {
      // 设置当前渲染的面
      // TODO: 需要FBO支持立方体贴图面选择

      BindShadowRenderContext(lightIdx, 0, faceIdx, 1);
      RenderSceneToShadowMap(context,
                             context.GetRenderQueue()->GetItems(RenderQueue::QueueType::Opaque));
    }
  }
}
void ShadowMapStage::RenderSpotShadowMaps(RenderContext &context)
{
  auto lightManager = context.GetLightManager();
  if (!lightManager.IsInitialized()) {
    m_Logger->warn("No LightManager available for spot shadow rendering");
    return;
  }

  // TODO: 获取聚光灯数量
  uint32_t spotLightCount = 0;  // lightManager.GetSpotLightCount();

  for (uint32_t lightIdx = 0; lightIdx < spotLightCount; lightIdx++) {
    // 绑定聚光灯阴影FBO
    if (lightIdx < m_SpotShadowFBOs.size()) {
      RenderCommand::Get().BindFrameBuffer(m_SpotShadowFBOs[lightIdx]);
    }

    // 绑定阴影渲染上下文
    BindShadowRenderContext(lightIdx, 0, 0, 2);  // 类型2=聚光灯

    // 渲染场景几何体
    auto renderQueue = context.GetRenderQueue();
    if (renderQueue) {
      // TODO: 实现聚光灯阴影的场景渲染
    }
  }

  m_Logger->debug("Rendered spot shadow maps for {} lights", spotLightCount);
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

void ShadowMapStage::UpdateShadowMatrices(RenderContext &context)
{
  // TODO: 实现阴影矩阵计算
  // 需要根据相机位置和光源参数计算：
  // - 方向光源的级联分割和视图投影矩阵
  // - 点光源的6个面的视图投影矩阵
  // - 聚光灯的视图投影矩阵

  m_Logger->info("Shadow matrix update - TODO: implement");
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

RuntimeTexturePtr ShadowMapStage::GetDirectionalShadowMap(uint32_t lightIndex,
                                                          uint32_t cascadeIndex) const
{
  if (lightIndex >= m_DirectionalShadowFBOs.size()) {
    m_Logger->warn("Requested directional shadow map for invalid light index: {}", lightIndex);
    return nullptr;
  }

  auto fbo = m_DirectionalShadowFBOs[lightIndex];
  if (!fbo) {
    m_Logger->warn("Directional shadow FBO for light {} is null", lightIndex);
    return nullptr;
  }

  // 获取深度附件（2D数组纹理）
  auto depthTexture = fbo->GetDepthAttachment();
  if (!depthTexture || !depthTexture->isValid()) {
    m_Logger->warn("Directional shadow FBO for light {} has invalid depth attachment", lightIndex);
    return nullptr;
  }

  // 验证级联索引
  if (cascadeIndex >= MAX_CASCADES) {
    m_Logger->warn("Requested cascade index {} exceeds maximum {}", cascadeIndex, MAX_CASCADES);
    return nullptr;
  }

  // 返回整个2D数组纹理，着色器通过层索引访问特定级联
  // 注意：这里返回的是包含所有级联的数组纹理，着色器使用cascadeIndex选择层
  return depthTexture;
}

RuntimeTexturePtr ShadowMapStage::GetPointShadowMap(uint32_t lightIndex) const
{
  if (lightIndex >= m_PointShadowFBOs.size()) {
    m_Logger->warn("Requested point shadow map for invalid light index: {}", lightIndex);
    return nullptr;
  }

  auto fbo = m_PointShadowFBOs[lightIndex];
  if (!fbo) {
    m_Logger->warn("Point shadow FBO for light {} is null", lightIndex);
    return nullptr;
  }

  // 获取深度附件（立方体贴图）
  RuntimeTexturePtr depthTexture = fbo->GetDepthAttachment();
  if (!depthTexture || !depthTexture->isValid()) {
    m_Logger->warn("Point shadow FBO for light {} has invalid depth attachment", lightIndex);
    return nullptr;
  }

  // 验证是否为立方体贴图
  if (depthTexture->getType != TextureTarget::TEXTURE_CUBE_MAP) {
    m_Logger->warn("Point shadow texture for light {} is not a cube map", lightIndex);
    return nullptr;
  }

  // 返回整个立方体贴图，着色器通过方向向量采样
  return depthTexture;
}

RuntimeTexturePtr ShadowMapStage::GetSpotShadowMap(uint32_t lightIndex) const
{
  if (lightIndex >= m_SpotShadowFBOs.size()) {
    m_Logger->warn("Requested spot shadow map for invalid light index: {}", lightIndex);
    return nullptr;
  }

  auto fbo = m_SpotShadowFBOs[lightIndex];
  if (!fbo) {
    m_Logger->warn("Spot shadow FBO for light {} is null", lightIndex);
    return nullptr;
  }

  // 获取深度附件（普通2D纹理）
  auto depthTexture = fbo->GetDepthAttachment();
  if (!depthTexture || !depthTexture->isValid()) {
    m_Logger->warn("Spot shadow FBO for light {} has invalid depth attachment", lightIndex);
    return nullptr;
  }

  // 返回2D阴影贴图
  return depthTexture;
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

  // 检查材质是否投射阴影
  // TODO: 需要材质系统支持阴影投射标志
  // if (!item.material->CastsShadows()) {
  //     return false;
  // }

  return true;
}

void ShadowMapStage::CalculateDirectionalShadowMatrices(RenderContext &context)
{
  // TODO: 实现方向光源级联阴影矩阵计算
  // 1. 根据相机视锥体和级联分割计算每个级联的包围盒
  // 2. 为每个方向光源和级联计算光空间视图投影矩阵
  // 3. 更新u_Shadow.directionalMatrices

  m_Logger->info("Directional shadow matrix calculation - TODO: implement");
}

void ShadowMapStage::CalculatePointShadowMatrices(RenderContext &context)
{
  // TODO: 实现点光源阴影矩阵计算
  // 1. 为每个点光源计算6个面的视图投影矩阵（立方体贴图）
  // 2. 更新u_Shadow.pointLightMatrices

  m_Logger->info("Point shadow matrix calculation - TODO: implement");
}

void ShadowMapStage::CalculateSpotShadowMatrices(RenderContext &context)
{
  // TODO: 实现聚光灯阴影矩阵计算
  // 1. 为每个聚光灯计算视图投影矩阵
  // 2. 更新u_Shadow.spotLightMatrices

  m_Logger->info("Spot shadow matrix calculation - TODO: implement");
}


}  // namespace mite