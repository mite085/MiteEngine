#ifndef MITE_FORWARD_STAGE
#define MITE_FORWARD_STAGE

#include "render_stage.h"

namespace mite {

/**
 * @brief 前向渲染阶段（接管原OpenGLRenderer的场景渲染功能）
 *
 * 职责：
 * 1. 渲染不透明、Alpha测试、透明物体
 * 2. 管理材质和Shader状态
 * 3. 处理渲染队列的排序和提交
 */
class ForwardStage : public RenderStage {
 public:
  ForwardStage();
  ~ForwardStage() override;

  // ---- 生命周期管理 ----
  void Initialize(RenderContext &context) override;
  void Execute(RenderContext &context) override;
  void Shutdown() override;

 private:
  // ---- 私有方法（接管原OpenGLRenderer的渲染逻辑）----
  /**
   * @brief 渲染透明物体队列
   */
  void RenderTransparentQueue(RenderContext &context);

  /**
   * @brief 验证渲染项的有效性
   */
  bool ValidateRenderableItem(const RenderableItem &item) const;

  // ---- 成员变量 ----
  std::shared_ptr<FrameBuffer> m_ForwardFrameBuffer;

  // ---- 渲染状态配置 ----
  std::shared_ptr<RenderState> m_TransparentState;

  // ---- 性能统计 ----
  size_t m_LastFrameTransparentCount = 0;
};

}  // namespace mite

#endif
