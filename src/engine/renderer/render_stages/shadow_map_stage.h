#ifndef MITE_SHADOWMAP_STAGE_H
#define MITE_SHADOWMAP_STAGE_H

#include "basic_shader/framebuffer.h"
#include "basic_shader/shader.h"
#include "light_core/light_manager.h"
#include "render_stage.h"

namespace mite {
/**
 * @brief ShadowMap渲染阶段 - 负责生成所有光源的阴影贴图
 *
 * 职责：
 * 1. 管理各种光源类型的阴影贴图Framebuffer
 * 2. 为方向光源生成级联阴影贴图
 * 3. 为点光源生成立方体贴图阴影
 * 4. 为聚光灯生成透视投影阴影贴图
 * 5. 输出阴影贴图到上下文供后续阶段使用
 */
class ShadowMapStage : public RenderStage {
 public:
  ShadowMapStage();
  ~ShadowMapStage() override;

  // ---- 生命周期管理 ----
  void Initialize() override;
  void Execute(RenderContext &context) override;
  void Shutdown() override;

  // ---- 阴影贴图访问接口 ----
  RuntimeTexturePtr GetDirectionalShadowMap() const;
  RuntimeTexturePtr GetPointShadowMap() const;
  RuntimeTexturePtr GetSpotShadowMap() const;

  // ---- 阴影质量配置与获取 ----
  void SetShadowQuality(ShadowQuality quality);
  void SetShadowFilter(ShadowFilter filter);
  void SetShadowBias(float bias, float normalBias);
  ShadowQuality GetShadowQuality() const { return m_ShadowQuality; }
  ShadowFilter GetShadowFilter() const { return m_ShadowFilter; }
  float GetShadowBias() const { return m_ShadowBias; }
  float GetNormalBias() const { return m_NormalBias; }

 private:
  // ---- 阴影贴图创建方法 ----
  void CreateDirectionalShadowMap();
  void CreatePointShadowMap();
  void CreateSpotShadowMap();

  // ---- 阴影渲染方法 ----
  void RenderDirectionalShadowMap(RenderContext &context,
                                  const std::vector<std::shared_ptr<Light>> &directionalLights);
  void RenderPointShadowMap(RenderContext &context,
                            const std::vector<std::shared_ptr<Light>> &pointLights);
  void RenderSpotShadowMap(RenderContext &context,
                           const std::vector<std::shared_ptr<Light>> &spotLights);

  // ---- 辅助方法 ----
  void SetupShadowRenderState();
  void BindShadowRenderContext(uint32_t lightIndex,
                               uint32_t cascadeIndex,
                               uint32_t faceIndex,
                               uint32_t shadowMapType);
  // 分层渲染辅助方法
  void BindFramebufferLayer(std::shared_ptr<FrameBuffer> fbo,
                            uint32_t attachmentPoint,
                            uint32_t layer);
  void BindFramebufferCubeFace(std::shared_ptr<FrameBuffer> fbo,
                               uint32_t attachmentPoint,
                               uint32_t layer,
                               uint32_t face);
  // 场景几何体渲染辅助方法
  void RenderSceneToShadowMap(RenderContext &context, const std::vector<RenderableItem> &items);
  // 存储shadow map到上下文
  void StoreShadowMapsToContext(RenderContext &context);

  // ---- 验证方法 ----
  void ValidateShadowInputs(RenderContext &context) const;
  bool ValidateShadowRenderableItem(const RenderableItem &item) const;

  // ---- 阴影贴图存储 ----
  std::shared_ptr<FrameBuffer> m_DirectionalShadowFBO;  // 方向光源2D数组纹理
  std::shared_ptr<FrameBuffer> m_PointShadowFBO;        // 点光源立方体贴图数组
  std::shared_ptr<FrameBuffer> m_SpotShadowFBO;         // 聚光灯2D数组纹理

  // 渲染状态
  std::shared_ptr<RenderState> m_ShadowRenderState;

  // 配置参数
  bool m_Initialized = false;
  ShadowQuality m_ShadowQuality = ShadowQuality::MEDIUM;
  ShadowFilter m_ShadowFilter = ShadowFilter::PCF;
  float m_ShadowBias = 0.005f;
  float m_NormalBias = 0.01f;
};
}  // namespace mite

#endif
