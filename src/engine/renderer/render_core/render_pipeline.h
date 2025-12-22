#ifndef MITE_RENDER_PIPELINE
#define MITE_RENDER_PIPELINE

#include "basic_instance/camera_instance.h"
#include "basic_shader/framebuffer.h"
#include "render_context.h"
#include "render_queue.h"
#include "render_stages/render_stage.h"

namespace mite {
/**
 * @brief 主渲染管线（接管原Renderer功能）
 *
 * 职责：
 * 1. 管理渲染阶段执行顺序
 * 2. 协调FrameBuffer和资源管理
 * 3. 提供统一的渲染接口
 */
class RenderPipeline {
 public:
  explicit RenderPipeline();
  virtual ~RenderPipeline() = default;

  // ---- 初始化与销毁 ----
  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;

  // ---- 帧控制 ----
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  // ---- 场景渲染 ----
  virtual void RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                           std::shared_ptr<CameraInstance> cameraInstance) = 0;

  // ---- 状态设置 ----
  virtual void SetClearColor(const glm::vec4 &color) = 0;

  // ---- 阶段管理 ----
  void AddStage(std::unique_ptr<RenderStage> stage,
                std::shared_ptr<OpenGLShader> shader);
  void SetStageEnabled(const std::string &stageName, bool enabled);
  RenderStage *GetStage(const std::string &stageName) const;

  // 外部访问上下文
  RenderContext &GetContext() { return *m_Context; }

 protected:
  std::unique_ptr<RenderContext> m_Context;
  std::vector<std::unique_ptr<RenderStage>> m_Stages;
  glm::vec4 m_ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
  Logger m_Logger;
};
}  // namespace mite

#endif
