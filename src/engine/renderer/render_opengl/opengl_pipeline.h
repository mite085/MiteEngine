#ifndef MITE_OPENGL_PIPELINE
#define MITE_OPENGL_PIPELINE

#include "basic_event/render_event.h"
#include "opengl_device.h"
#include "render_core/render_command.h"
#include "render_core/render_context.h"
#include "render_core/render_pipeline.h"

namespace mite {
/**
 * @brief OpenGL渲染管线实现
 *
 * 接管原OpenGLRenderer的所有功能，并扩展为多阶段架构
 */
class OpenGLPipeline : public RenderPipeline {
 public:
  explicit OpenGLPipeline();
  ~OpenGLPipeline() override;

  // ---- 初始化与销毁 ----
  void Initialize() override;
  void Shutdown() override;

  // ---- 帧控制 ----
  void BeginFrame() override;
  void EndFrame() override;

  // ---- 场景渲染 ----
  void RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                   CameraInstance &cameraInstance) override;

  // ---- 状态设置 ----
  void SetClearColor(const glm::vec4 &color) override;

  //// ---- FBO接口（FBO由各个Stage管理，此处弃用） ----
  // std::shared_ptr<FrameBuffer> GetMainFrameBuffer() const override;
  // std::shared_ptr<FrameBuffer> GetDisplayFrameBuffer() const override;
  // void CreateDefaultFrameBuffer();
  // void SwapFrameBuffers();

 private:
  // ---- 事件处理 ----
  void OnViewPortResize(ViewPortResizeEvent &event);  // 消费Viewport尺寸变化事件

  // ---- 成员变量 ----
  std::unique_ptr<RenderContext> m_Context;
  bool m_IsRenderingScene = false;
  SubscriptionGroup m_EventSubscriptions;

  // ---- Size管理 ----
  bool m_ShouldResize = false;
  glm::uvec2 m_PendingSize = {1280, 720};  // 默认尺寸
};
}  // namespace mite

#endif
