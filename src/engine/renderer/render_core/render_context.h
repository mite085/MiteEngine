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
  CameraInstance& GetMainCameraInstance() const { return *m_MainCameraInstance; }
  LightManager &GetLightManager() const { return LightManager::Get(); }

  // ---- 临时资源管理（现阶段尽量所有资源明确定义，待后续启用临时资源） ----
  // template<typename T>
  // void SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource);
  // template<typename T> std::shared_ptr<T> GetTemporaryResource(const std::string &name) const;
  // void ClearTemporaryResources();

  // ---- 上下文状态 ----
  bool IsValid() const;
  void Validate() const;
  void DebugTextureInfo() const;

 private:
  // ---- 私有方法 ----
  /**
   * @brief 消费事件，注册相机/网格/材质实例（设置所有现有着色器的UBO绑定）
   * (使用固定的绑定点执行显示绑定，无需手动管理)
   */
  //void OnCameraInstanceCreated(CameraInstanceCreateEvent &event);
  //void OnMeshInstanceCreated(MeshInstanceCreateEvent &event);
  //void OnMaterialInstanceCreated(MaterialInstanceCreateEvent &event);
  //void OnLightSSBOCreated(LightSSBOCreateEvent &event);
  ///**
  // * @brief 为新着色器设置所有已注册实例的UBO绑定
  // (使用固定的绑定点执行显示绑定，无需手动管理)
  // */
  //void SetupShaderBindingsForNewShader(const std::string &stageName,
  //                                     std::shared_ptr<OpenGLShader> shader);
  ///**
  // * @brief 为新实例设置所有已注册着色器的UBO绑定
  // (使用固定的绑定点执行显示绑定，无需手动管理)
  // */
  //template<typename T> void SetupShaderBindingsForNewInstance(std::shared_ptr<T> instance)
  //{
  //  for (auto &[stageName, shader] : m_StageShaders) {
  //    instance->SetupShaderBinding(shader);
  //    m_Logger->debug("Setting up shader binding for instance in stage: {}", stageName);
  //  }
  //}

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
  std::shared_ptr<LightShaderStorgeBuffer> m_LightSSBO;  // LightSSBO包含了所有光照信息，无需按照Vector存储

  // ---- 窗口尺寸 ----
  glm::uvec2 m_ViewportSize = {1280, 720};  // 默认尺寸

  // ---- 分层纹理存储 ----
  std::array<RuntimeTexturePtr, GBuffer::TEXTURE_COUNT> m_GBufferTextures;
  std::unordered_map<uintptr_t, RuntimeTexturePtr> m_ShadowMapTextures;
  std::unordered_map<std::string, RuntimeTexturePtr> m_RenderTargets;

  // ---- 临时资源存储（现阶段尽量所有资源明确定义，待后续启用临时资源） ----
  // std::unordered_map<std::string, std::shared_ptr<void>> m_TemporaryResources;

  Logger m_Logger;
  SubscriptionGroup m_EventSubscription;
};

//// 临时资源管理模板实现（现阶段尽量所有资源明确定义，待后续启用临时资源）
// template<typename T>
// void RenderContext::SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource)
//{
//   if (name.empty()) {
//     m_Logger->warn("Attempted to set temporary resource with empty name");
//     return;
//   }
//
//   // 转换为void指针存储，但保持类型安全通过模板管理
//   std::shared_ptr<void> voidResource = std::static_pointer_cast<void>(resource);
//   m_TemporaryResources[name] = voidResource;
//
//   // m_Logger->debug("Set temporary resource: {}", name);
// }
//
// template<typename T>
// std::shared_ptr<T> RenderContext::GetTemporaryResource(const std::string &name) const
//{
//   auto it = m_TemporaryResources.find(name);
//   if (it == m_TemporaryResources.end()) {
//     m_Logger->debug("Temporary resource not found: {}", name);
//     return nullptr;
//   }
//
//   try {
//     // 尝试转换回原始类型
//     std::shared_ptr<T> typedResource = std::static_pointer_cast<T>(it->second);
//     return typedResource;
//   }
//   catch (const std::bad_cast &e) {
//     m_Logger->error("Failed to cast temporary resource '{}': {}", name, e.what());
//     return nullptr;
//   }
// }
}  // namespace mite

#endif
