#include "render_context.h"

namespace mite {
RenderContext::RenderContext() : m_Logger(LoggerSystem::CreateModuleLogger("Mite Render Context"))
{
  // 初始化GBuffer纹理数组
  m_GBufferTextures.fill(nullptr);

  m_Logger->info("Render Context created");
}

RenderContext::~RenderContext()
{
  ClearTemporaryResources();
  ClearTextures();
  m_Logger->info("Render Context destroyed");
}

void RenderContext::SetSceneData(std::shared_ptr<RenderQueue> renderQueue,
                                 CameraInstance &cameraInstance)
{
  m_RenderQueue = renderQueue;
  m_CameraInstance = std::ref(cameraInstance);

  // m_Logger->debug("Set scene data - RenderQueue: {}, Camera position: ({}, {}, {})",
  //                 m_RenderQueue ? "valid" : "null",
  //                 m_CameraPosition.x,
  //                 m_CameraPosition.y,
  //                 m_CameraPosition.z);
}

// ---- 分层纹理管理实现 ----
void RenderContext::SetGBufferTexture(RuntimeTexturePtr texture)
{
  if (texture && texture->isValid()) {
    uint32_t index = GBuffer::TextureTypeToIndex.at(texture->getType());

    // 存入GBuffer纹理管理中
    if (index >= 0 && index < GBuffer::TEXTURE_COUNT) {
      m_GBufferTextures[index] = texture;
      m_Logger->debug("Set GBuffer texture [{}]", static_cast<int>(index));
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
  m_Logger->debug("Set RenderTarget: {}", name);
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

void RenderContext::ClearTemporaryResources()
{
  size_t count = m_TemporaryResources.size();
  m_TemporaryResources.clear();

  if (count > 0) {
    m_Logger->debug("Cleared {} temporary resources", count);
  }
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
}  // namespace mite