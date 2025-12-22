#ifndef MITE_BLEND_STAGE_H
#define MITE_BLEND_STAGE_H

#include "basic_shader/framebuffer.h"
#include "basic_shader/shader.h"
#include "render_stage.h"

namespace mite {
/**
 * @brief 混合阶段 - 将Deferred Lighting结果和Forward半透明结果进行Alpha混合
 *
 * 职责：
 * 1. 管理最终输出的Framebuffer
 * 2. 从RenderContext获取Deferred Lighting和Forward结果纹理
 * 3. 执行全屏四边形混合计算
 * 4. 输出最终渲染结果
 */
class BlendStage : public RenderStage {
 public:
  BlendStage();
  ~BlendStage() override;

  // ---- 生命周期管理 ----
  void Initialize(RenderContext &context) override;
  void Execute(RenderContext &context) override;
  void Shutdown() override;

  // ---- FBO访问接口 ----
  std::shared_ptr<FrameBuffer> GetBlendFramebuffer() const {
    return m_BlendFBO;
  }

 private:
  // ---- 私有方法 ----
  void CreateBlendFramebuffer();
  void SetupBlendRenderState();

  // ---- 绑定方法 ----
  void BindInputTextures(RenderContext &context);

  // ---- 验证方法 ----
  void ValidateInputs(RenderContext &context) const;
  void ValidateBlendFramebuffer(const glm::vec2 &viewportSize);

  // ---- 成员变量 ----
  std::shared_ptr<FrameBuffer> m_BlendFBO;  // 最终输出Framebuffer

  // 渲染状态
  std::shared_ptr<RenderState> m_BlendState;

  // 配置参数
  bool m_Initialized = false;
};
}  // namespace mite

#endif  // MITE_BLEND_STAGE_H
