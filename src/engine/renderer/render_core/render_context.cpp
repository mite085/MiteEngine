#include "render_context.h"

namespace mite {
RenderContext::RenderContext() : m_Logger(LoggerSystem::CreateModuleLogger("Mite Render Context"))
{
  m_Logger->info("Render Context created");
}

RenderContext::~RenderContext()
{
  // ClearTemporaryResources();
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

// ---- 阴影贴图管理实现 ----
void RenderContext::SetShadowMapTexture(LightType type, RuntimeTexturePtr texture)
{
  if (texture && texture->IsValid()) {
    m_ShadowMapTextures[type] = texture;
    m_Logger->debug("Set shadow map texture for type: {}", static_cast<int>(type));
  }
  else {
    m_Logger->warn("Attempted to set invalid shadow map texture for type: {}",
                   static_cast<int>(type));
  }
}
RuntimeTexturePtr RenderContext::GetShadowMapTexture(LightType type) const
{
  auto it = m_ShadowMapTextures.find(type);
  if (it != m_ShadowMapTextures.end()) {
    return it->second;
  }
  m_Logger->debug("Shadow map texture not found for type: {}", static_cast<int>(type));
  return nullptr;
}
bool RenderContext::HasShadowMapTexture(LightType type) const
{
  auto it = m_ShadowMapTextures.find(type);
  return it != m_ShadowMapTextures.end() && it->second && it->second->IsValid();
}
void RenderContext::SetRenderTarget(const std::string &name, RuntimeTexturePtr texture)
{
  if (name.empty()) {
    m_Logger->warn("Attempted to set RenderTarget with empty name");
    return;
  }
  m_RenderTargets[name] = texture;
  // m_Logger->debug("Set RenderTarget: {}", name);
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
  m_ShadowMapTextures.clear();
  m_RenderTargets.clear();
  m_Logger->debug("Cleared all texture mappings");
}

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
}  // namespace mite