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
  RuntimeTexturePtr GetDirectionalShadowMap(uint32_t lightIndex, uint32_t cascadeIndex) const;
  RuntimeTexturePtr GetPointShadowMap(uint32_t lightIndex) const;
  RuntimeTexturePtr GetSpotShadowMap(uint32_t lightIndex) const;

 private:
  // ---- 阴影贴图创建方法 ----
  void CreateDirectionalShadowMaps();
  void CreatePointShadowMaps();
  void CreateSpotShadowMaps();

  // ---- 阴影渲染方法 ----
  void RenderDirectionalShadowMaps(RenderContext &context);
  void RenderPointShadowMaps(RenderContext &context);
  void RenderSpotShadowMaps(RenderContext &context);

  // ---- 辅助方法 ----
  void SetupShadowRenderState();
  void UpdateShadowMatrices(RenderContext &context);
  void BindShadowRenderContext(uint32_t lightIndex,
                               uint32_t cascadeIndex,
                               uint32_t faceIndex,
                               uint32_t shadowMapType);
  // 场景几何体渲染辅助方法
  void RenderSceneToShadowMap(RenderContext &context, const std::vector<RenderableItem> &items);
  // 阴影矩阵计算辅助方法
  void CalculateDirectionalShadowMatrices(RenderContext &context);
  void CalculatePointShadowMatrices(RenderContext &context);
  void CalculateSpotShadowMatrices(RenderContext &context);
  // 存储shadow map到上下文
  void StoreShadowMapsToContext(RenderContext &context);

  // ---- 验证方法 ----
  void ValidateShadowInputs(RenderContext &context) const;
  bool ValidateShadowRenderableItem(const RenderableItem &item) const;

  // ---- 阴影贴图存储 ----
  std::vector<std::shared_ptr<FrameBuffer>> m_DirectionalShadowFBOs;  // 方向光源级联阴影
  std::vector<std::shared_ptr<FrameBuffer>> m_PointShadowFBOs;        // 点光源立方体贴图
  std::vector<std::shared_ptr<FrameBuffer>> m_SpotShadowFBOs;         // 聚光灯阴影

  // 渲染状态
  std::shared_ptr<RenderState> m_ShadowRenderState;

  // 配置参数
  bool m_Initialized = false;
  uint32_t m_ShadowMapSize = 2048;  // 阴影贴图分辨率

  // 阴影UBO数据（需要在C++端维护）
  // TODO: 需要ShadowUniformBuffer和ShadowRenderContextUniformBuffer的实现
};

}  // namespace mite

#endif
