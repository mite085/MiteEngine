#ifndef MITE_RENDER_CONTEXT
#define MITE_RENDER_CONTEXT

#include "basic_data/runtime_texture.h"
#include "basic_event/instance_event.h"
#include "basic_instance/camera_instance.h"
#include "basic_instance/material_instance.h"
#include "basic_instance/mesh_instance.h"
#include "basic_shader/gbuffer.h"
#include "light_core/light_manager.h"
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
  void SetSceneData(std::shared_ptr<RenderQueue> renderQueue,
                    std::shared_ptr<CameraInstance> cameraInstance);

  // ---- 着色器阶段管理 ----
  /**
   * @brief 注册渲染阶段着色器
   * @param stageName 阶段名称（如"GBufferStage", "DeferredLightingStage"等）
   * @param shader 着色器对象
   */
  void RegisterStageShader(const std::string &stageName, std::shared_ptr<OpenGLShader> shader);
  /**
   * @brief 获取指定阶段的着色器
   */
  std::shared_ptr<OpenGLShader> GetStageShader(const std::string &stageName) const;
  /**
   * @brief 获取所有已注册的阶段着色器
   */
  const std::unordered_map<std::string, std::shared_ptr<OpenGLShader>> &GetAllStageShaders() const;

  // ---- 窗口尺寸管理 ----
  void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportSize = {width, height}; }
  const glm::vec2 &GetViewportSize() const { return m_ViewportSize; }
  uint32_t GetViewportWidth() const { return static_cast<uint32_t>(m_ViewportSize.x); }
  uint32_t GetViewportHeight() const {return static_cast<uint32_t>(m_ViewportSize.y); }
  float GetViewportAspectRatio() const
  {
    return m_ViewportSize.y > 0 ? static_cast<float>(m_ViewportSize.x) / m_ViewportSize.y : 1.0f;
  }

  // ---- 分层纹理管理 ----

  // ShadowMap纹理管理（动态数量，按类型存储）
  void SetShadowMapTexture(LightType type, RuntimeTexturePtr texture);
  RuntimeTexturePtr GetShadowMapTexture(LightType type) const;
  bool HasShadowMapTexture(LightType type) const;

  // RenderTarget纹理管理（自定义命名）
  void SetRenderTarget(const std::string &name, RuntimeTexturePtr texture);
  RuntimeTexturePtr GetRenderTarget(const std::string &name) const;

  // 清空所有纹理（每帧开始时调用）
  void ClearTextures();

  // ---- 数据访问接口 ----

  // 场景数据
  std::shared_ptr<RenderQueue> GetRenderQueue() const { return m_RenderQueue; }
  std::shared_ptr<CameraInstance> GetMainCameraInstance() const { return m_MainCameraInstance; }
  LightManager &GetLightManager() const { return LightManager::Get(); }

  // ---- 上下文状态 ----
  bool IsValid() const;
  void Validate() const;

 private:
  // ---- 私有方法 ----

  // ---- 场景数据 ----
  std::shared_ptr<RenderQueue> m_RenderQueue;
  std::shared_ptr<CameraInstance> m_MainCameraInstance;

  // ---- 着色器阶段管理 ----
  std::unordered_map<std::string, std::shared_ptr<OpenGLShader>> m_StageShaders;

  // ---- 渲染实例存储 ----
  // 注意：仅用于确保所有实例管理的UBO在所有StageShader中执行注册
  std::vector<std::shared_ptr<CameraInstance>> m_CameraInstances;
  std::vector<std::shared_ptr<MeshInstance>> m_MeshInstances;
  std::vector<std::shared_ptr<MaterialInstance>> m_MaterialInstances;
  std::shared_ptr<LightShaderStorgeBuffer>
      m_LightSSBO;  // LightSSBO包含了所有光照信息，无需按照Vector存储

  // ---- 窗口尺寸 ----
  glm::vec2 m_ViewportSize = {1280, 720};  // 默认尺寸

  // ---- GBuffer存储 ----
  std::shared_ptr<GBuffer> m_GBuffer = nullptr;

  // ---- 分层纹理存储 ----
  std::unordered_map<LightType, RuntimeTexturePtr> m_ShadowMapTextures;
  std::unordered_map<std::string, RuntimeTexturePtr> m_RenderTargets;

  Logger m_Logger;
  SubscriptionGroup m_EventSubscription;
};


}  // namespace mite

#endif
