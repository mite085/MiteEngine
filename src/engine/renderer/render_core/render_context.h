#ifndef MITE_RENDER_CONTEXT
#define MITE_RENDER_CONTEXT

#include "basic_data/runtime_texture.h"
#include "basic_instance/camera_instance.h"
#include "basic_shader/gbuffer.h"
#include "render_queue.h"

namespace mite {
/**
 * @brief 渲染上下文（统一数据传递和资源共享）
 *
 * 职责：
 * 1. 封装渲染所需的所有数据（场景、相机、FrameBuffer等）
 * 2. 提供阶段间的数据共享机制
 * 3. 管理临时渲染资源
 * 4. 提供分层纹理管理（GBuffer、ShadowMap、RenderTarget）
 * 5. 管理着色器阶段注册和UBO绑定自动设置
 */
class RenderContext {
 public:
  explicit RenderContext();
  ~RenderContext();

  // ---- 场景数据设置 ----
  void SetSceneData(std::shared_ptr<RenderQueue> renderQueue, CameraInstance &cameraInstance);

  // ---- 窗口尺寸管理 ----
  void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportSize = {width, height}; }
  const glm::uvec2 &GetViewportSize() const { return m_ViewportSize; }
  uint32_t GetViewportWidth() const { return m_ViewportSize.x; }
  uint32_t GetViewportHeight() const { return m_ViewportSize.y; }
  float GetViewportAspectRatio() const
  {
    return m_ViewportSize.y > 0 ? static_cast<float>(m_ViewportSize.x) / m_ViewportSize.y : 1.0f;
  }

  // ---- 分层纹理管理 ----

  // GBuffer纹理管理（固定数量）
  void SetGBufferTexture(RuntimeTexturePtr texture);
  RuntimeTexturePtr GetGBufferTexture(RuntimeTextureType type) const;

  // ShadowMap纹理管理（动态数量）
  void SetShadowMapTexture(uint32_t lightId, uint32_t shadowIndex, RuntimeTexturePtr texture);
  RuntimeTexturePtr GetShadowMapTexture(uint32_t lightId, uint32_t shadowIndex) const;

  // RenderTarget纹理管理（自定义命名）
  void SetRenderTarget(const std::string &name, RuntimeTexturePtr texture);
  RuntimeTexturePtr GetRenderTarget(const std::string &name) const;

  // 清空所有纹理（每帧开始时调用）
  void ClearTextures();

  // ---- 数据访问接口 ----

  // 场景数据
  std::shared_ptr<RenderQueue> GetRenderQueue() const { return m_RenderQueue; }
  CameraInstance &GetCameraInstance() const
  {
    if (m_CameraInstance.has_value()) {
      return m_CameraInstance.value().get();
    }
    throw std::runtime_error("Reference not set");
  }

  // ---- 临时资源管理 ----
  template<typename T>
  void SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource);
  template<typename T> std::shared_ptr<T> GetTemporaryResource(const std::string &name) const;
  void ClearTemporaryResources();

  // ---- 上下文状态 ----
  bool IsValid() const;
  void Validate() const;
  void DebugTextureInfo() const;

 private:
  // ---- 场景数据 ----
  std::shared_ptr<RenderQueue> m_RenderQueue;
  std::optional<std::reference_wrapper<CameraInstance>>
      m_CameraInstance;  // 支持空引用的相机实例引用

  // ---- 窗口尺寸 ----
  glm::uvec2 m_ViewportSize = {1280, 720};  // 默认尺寸

  // ---- 分层纹理存储 ----
  std::array<RuntimeTexturePtr, GBuffer::TEXTURE_COUNT> m_GBufferTextures;
  std::unordered_map<uintptr_t, RuntimeTexturePtr> m_ShadowMapTextures;
  std::unordered_map<std::string, RuntimeTexturePtr> m_RenderTargets;

  // ---- 临时资源存储 ----
  std::unordered_map<std::string, std::shared_ptr<void>> m_TemporaryResources;

  Logger m_Logger;
};

// 模板实现
template<typename T>
void RenderContext::SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource)
{
  if (name.empty()) {
    m_Logger->warn("Attempted to set temporary resource with empty name");
    return;
  }

  // 转换为void指针存储，但保持类型安全通过模板管理
  std::shared_ptr<void> voidResource = std::static_pointer_cast<void>(resource);
  m_TemporaryResources[name] = voidResource;

  // m_Logger->debug("Set temporary resource: {}", name);
}

template<typename T>
std::shared_ptr<T> RenderContext::GetTemporaryResource(const std::string &name) const
{
  auto it = m_TemporaryResources.find(name);
  if (it == m_TemporaryResources.end()) {
    m_Logger->debug("Temporary resource not found: {}", name);
    return nullptr;
  }

  try {
    // 尝试转换回原始类型
    std::shared_ptr<T> typedResource = std::static_pointer_cast<T>(it->second);
    return typedResource;
  }
  catch (const std::bad_cast &e) {
    m_Logger->error("Failed to cast temporary resource '{}': {}", name, e.what());
    return nullptr;
  }
}
}  // namespace mite

#endif
