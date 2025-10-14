#ifndef MITE_DEFERRED_LIGHTING_STAGE_H
#define MITE_DEFERRED_LIGHTING_STAGE_H

#include "basic_shader/framebuffer.h"
#include "basic_shader/shader.h"
#include "light_core/light_manager.h"
#include "render_stage.h"

namespace mite {
/**
 * @brief 延迟光照阶段 - 基于G-Buffer和SSBO光源数据进行光照计算
 *
 * 职责：
 * 1. 管理光照计算专用的Framebuffer（类似GBuffer模式）
 * 2. 从RenderContext获取G-Buffer纹理进行光照计算
 * 3. 通过LightManager绑定光源SSBO数据
 * 4. 执行全屏四边形光照计算
 * 5. 输出最终光照结果到光照Framebuffer
 */
class DeferredLightingStage : public RenderStage {
 public:
  DeferredLightingStage();
  ~DeferredLightingStage() override;

  // ---- 生命周期管理 ----
  void Initialize() override;
  void Execute(RenderContext &context) override;
  void Shutdown() override;

  // ---- FBO访问接口 ----
  std::shared_ptr<FrameBuffer> GetLightingFramebuffer() const { return m_LightingFBO; }
  RuntimeTexturePtr GetLightingOutputTexture() const;

 private:
  // ---- 私有方法 ----
  void CreateLightingFramebuffer();
  void SetupLightingRenderState();

  // ---- 绑定方法 ----
  void BindGBufferTextures(RenderContext &context, std::shared_ptr<OpenGLShader> lightingShader);
  void BindLightSSBOData(RenderContext &context, std::shared_ptr<OpenGLShader> lightingShader);

  // ---- 验证方法 ----
  void ValidateInputs(RenderContext &context) const;
  void ValidateLightingFramebuffer(const glm::uvec2 &viewportSize);

  // ---- 阴影纹理绑定 ----
  void BindShadowMapTextures(RenderContext &context);

  // ---- 成员变量 ----
  std::shared_ptr<FrameBuffer> m_LightingFBO;  // 光照输出Framebuffer

  // 渲染状态
  std::shared_ptr<RenderState> m_LightingState;

  // 配置参数
  bool m_Initialized = false;
  bool m_EnableShadows = false;

  // 阴影相关
  static constexpr uint32_t MAX_SHADOW_MAPS = 16;
  uint32_t m_NextShadowTextureUnit = 8;
};
}  // namespace mite

#endif
