#include "render_context.h"

namespace mite {
RenderContext::RenderContext() : m_Logger(LoggerSystem::CreateModuleLogger("Mite Render Context"))
{
  // 初始化GBuffer纹理数组
  m_GBufferTextures.fill(nullptr);

  // 订阅Instance创建事件
  //m_EventSubscription.SubscribeImmediate<CameraInstanceCreateEvent>(
  //    BIND_DISPATCH_FN(OnCameraInstanceCreated));
  //m_EventSubscription.SubscribeImmediate<MeshInstanceCreateEvent>(
  //    BIND_DISPATCH_FN(OnMeshInstanceCreated));
  //m_EventSubscription.SubscribeImmediate<MaterialInstanceCreateEvent>(
  //    BIND_DISPATCH_FN(OnMaterialInstanceCreated));
  //m_EventSubscription.SubscribeImmediate<LightSSBOCreateEvent>(
  //    BIND_DISPATCH_FN(OnLightSSBOCreated));

  m_Logger->info("Render Context created");
}

RenderContext::~RenderContext()
{
  //ClearTemporaryResources();
  ClearTextures();
  m_Logger->info("Render Context destroyed");
}

void RenderContext::SetSceneData(std::shared_ptr<RenderQueue> renderQueue,
                                 std::shared_ptr<CameraInstance> cameraInstance)
{
  m_RenderQueue = renderQueue;
  m_MainCameraInstance = cameraInstance;

  // m_Logger->debug("Set scene data - RenderQueue: {}, Camera position: ({}, {}, {})",
  //                 m_RenderQueue ? "valid" : "null",
  //                 m_CameraPosition.x,
  //                 m_CameraPosition.y,
  //                 m_CameraPosition.z);
}

// ---- 着色器阶段管理实现 ----
void RenderContext::RegisterStageShader(const std::string &stageName,
                                        std::shared_ptr<OpenGLShader> shader)
{
  if (!shader) {
    m_Logger->warn("Attempted to register null shader for stage: {}", stageName);
    return;
  }
  if (shader->GetProgramId() == 0) {
    m_Logger->warn("Shader for stage {} is not linked, UBO bindings may fail", stageName);
  }
  m_StageShaders[stageName] = shader;
  m_Logger->debug("Registered shader for stage: {}", stageName);

  // 为新着色器设置所有已注册实例的UBO绑定
  //SetupShaderBindingsForNewShader(stageName, shader);
}
std::shared_ptr<OpenGLShader> RenderContext::GetStageShader(const std::string &stageName) const
{
  auto it = m_StageShaders.find(stageName);
  if (it != m_StageShaders.end()) {
    return it->second;
  }
  m_Logger->debug("Stage shader not found: {}", stageName);
  return nullptr;
}
const std::unordered_map<std::string, std::shared_ptr<OpenGLShader>> &RenderContext::
    GetAllStageShaders() const
{
  return m_StageShaders;
}

// ---- 分层纹理管理实现 ----
void RenderContext::SetGBufferTexture(RuntimeTexturePtr texture)
{
  if (texture && texture->isValid()) {
    uint32_t index = GBuffer::TextureTypeToIndex.at(texture->getType());

    // 存入GBuffer纹理管理中
    if (index >= 0 && index < GBuffer::TEXTURE_COUNT) {
      m_GBufferTextures[index] = texture;
      //m_Logger->debug("Set GBuffer texture [{}]", static_cast<int>(index));
    }
    else {
      m_Logger->warn("Invalid GBuffer index: {}", static_cast<int>(index));
    }
  }
}
RuntimeTexturePtr RenderContext::GetGBufferTexture(RuntimeTextureType type) const
{
  uint32_t index = GBuffer::TextureTypeToIndex.at(type);
  if (index >= 0 && index < GBuffer::TEXTURE_COUNT) {
    return m_GBufferTextures[index];
  }
  m_Logger->warn("Invalid GBuffer index: {}", static_cast<int>(index));
  return nullptr;
}
void RenderContext::SetShadowMapTexture(uint32_t lightId,
                                        uint32_t shadowIndex,
                                        RuntimeTexturePtr texture)
{
  uint64_t key = (static_cast<uint64_t>(lightId) << 32) | shadowIndex;
  m_ShadowMapTextures[key] = texture;
  m_Logger->debug("Set ShadowMap texture [light:{}, index:{}]", lightId, shadowIndex);
}
RuntimeTexturePtr RenderContext::GetShadowMapTexture(uint32_t lightId, uint32_t shadowIndex) const
{
  uint64_t key = (static_cast<uint64_t>(lightId) << 32) | shadowIndex;
  auto it = m_ShadowMapTextures.find(key);
  if (it != m_ShadowMapTextures.end()) {
    return it->second;
  }
  m_Logger->debug("ShadowMap texture not found [light:{}, index:{}]", lightId, shadowIndex);
  return nullptr;
}
void RenderContext::SetRenderTarget(const std::string &name, RuntimeTexturePtr texture)
{
  if (name.empty()) {
    m_Logger->warn("Attempted to set RenderTarget with empty name");
    return;
  }
  m_RenderTargets[name] = texture;
  //m_Logger->debug("Set RenderTarget: {}", name);
}
RuntimeTexturePtr RenderContext::GetRenderTarget(const std::string &name) const
{
  auto it = m_RenderTargets.find(name);
  if (it != m_RenderTargets.end()) {
    return it->second;
  }
  m_Logger->debug("RenderTarget not found: {}", name);
  return nullptr;
}
void RenderContext::ClearTextures()
{
  // 清空所有纹理映射（注意：这里清空的是映射，不是纹理本身）
  m_GBufferTextures.fill(nullptr);
  m_ShadowMapTextures.clear();
  m_RenderTargets.clear();
  m_Logger->debug("Cleared all texture mappings");
}

// ---- 临时资源管理（现阶段尽量所有资源明确定义，待后续启用临时资源） ----
//void RenderContext::ClearTemporaryResources()
//{
//  size_t count = m_TemporaryResources.size();
//  m_TemporaryResources.clear();
//
//  if (count > 0) {
//    m_Logger->debug("Cleared {} temporary resources", count);
//  }
//}

bool RenderContext::IsValid() const
{
  return m_RenderQueue != nullptr;
}

void RenderContext::Validate() const
{
  if (!m_RenderQueue) {
    throw std::runtime_error("RenderContext validation failed: No render queue");
  }
  m_Logger->debug("RenderContext validation passed");
}

void RenderContext::DebugTextureInfo() const
{
  size_t gbufferCount = std::count_if(m_GBufferTextures.begin(),
                                      m_GBufferTextures.end(),
                                      [](auto &ptr) { return ptr != nullptr; });

  m_Logger->info(
      "Texture Info - GBuffer: {}/{}, ShadowMap: {}, RenderTargets: {}, Total by handle: {}",
      gbufferCount,
      m_GBufferTextures.size(),
      m_ShadowMapTextures.size(),
      m_RenderTargets.size());
}
// ---- 渲染实例事件消费 ----
//void RenderContext::OnCameraInstanceCreated(CameraInstanceCreateEvent &event)
//{
//  std::shared_ptr<CameraInstance> cameraInstance = event.GetInstance();
//
//  if (!cameraInstance) {
//    m_Logger->warn("Attempted to register null camera instance");
//    return;
//  }
//  m_CameraInstances.push_back(cameraInstance);
//  m_Logger->debug("Registered camera instance");
//
//  // 为新实例设置所有已注册着色器的UBO绑定
//  SetupShaderBindingsForNewInstance<CameraInstance>(cameraInstance);
//
//  // 阻断事件传播
//  event.SetResult(EventResult::HandledAndStop);
//  return;
//}
//void RenderContext::OnMeshInstanceCreated(MeshInstanceCreateEvent &event)
//{
//  std::shared_ptr<MeshInstance> meshInstance = event.GetInstance();
//
//  if (!meshInstance) {
//    m_Logger->warn("Attempted to register null mesh instance");
//    return;
//  }
//  m_MeshInstances.push_back(meshInstance);
//  m_Logger->debug("Registered mesh instance");
//
//  // 为新实例设置所有已注册着色器的UBO绑定
//  SetupShaderBindingsForNewInstance<MeshInstance>(meshInstance);
//
//  // 阻断事件传播
//  event.SetResult(EventResult::HandledAndStop);
//  return;
//}
//void RenderContext::OnMaterialInstanceCreated(MaterialInstanceCreateEvent &event)
//{
//  std::shared_ptr<MaterialInstance> materialInstance = event.GetInstance();
//
//  if (!materialInstance) {
//    m_Logger->warn("Attempted to register null material instance");
//    return;
//  }
//  m_MaterialInstances.push_back(materialInstance);
//  m_Logger->debug("Registered material instance");
//
//  // 为新实例设置所有已注册着色器的UBO绑定
//  SetupShaderBindingsForNewInstance<MaterialInstance>(materialInstance);
//
//  // 阻断事件传播
//  event.SetResult(EventResult::HandledAndStop);
//  return;
//}
//void RenderContext::OnLightSSBOCreated(LightSSBOCreateEvent &event)
//{
//  std::shared_ptr<LightShaderStorgeBuffer> lightSSBO = event.GetSSBO();
//
//  if (!lightSSBO) {
//    m_Logger->warn("Attempted to register null light ssbo");
//    return;
//  }
//  m_LightSSBO = lightSSBO;
//  m_Logger->debug("Registered material instance");
//
//  // 为新实例设置所有已注册着色器的UBO绑定
//  SetupShaderBindingsForNewInstance<LightShaderStorgeBuffer>(lightSSBO);
//
//  // 阻断事件传播
//  event.SetResult(EventResult::HandledAndStop);
//  return;
//}

//void RenderContext::SetupShaderBindingsForNewShader(const std::string &stageName,
//                                                    std::shared_ptr<OpenGLShader> shader)
//{
//  // 为所有相机实例设置绑定
//  for (auto &cameraInstance : m_CameraInstances) {
//    cameraInstance->SetupShaderBinding(shader);
//  }
//  // 为所有网格实例设置绑定
//  for (auto &meshInstance : m_MeshInstances) {
//    meshInstance->SetupShaderBinding(shader);
//  }
//  // 为所有材质实例设置绑定
//  for (auto &materialInstance : m_MaterialInstances) {
//    materialInstance->SetupShaderBinding(shader);
//  }
//  // 为光照SSBO设置绑定
//  if (m_LightSSBO)
//    m_LightSSBO->SetupShaderBinding(shader);
//
//  m_Logger->info("Setup shader bindings for stage: {} ({} cameras, {} meshes, {} materials)",
//                 stageName,
//                 m_CameraInstances.size(),
//                 m_MeshInstances.size(),
//                 m_MaterialInstances.size());
//}
}  // namespace mite