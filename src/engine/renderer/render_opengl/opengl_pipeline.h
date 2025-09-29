#ifndef MITE_OPENGL_PIPELINE
#define MITE_OPENGL_PIPELINE

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

  // ---- 帧控制（接管原OpenGLRenderer）----
  void BeginFrame() override;
  void EndFrame() override;

  // ---- 场景渲染（接管原OpenGLRenderer）----
  void RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                   const glm::mat4 viewMatrix,
                   const glm::mat4 projectionMatrix) override;

  // ---- 状态设置（接管原OpenGLRenderer）----
  void SetClearColor(const glm::vec4 &color) override;

  // ---- UI接口（接管原OpenGLRenderer）----
  std::shared_ptr<FrameBuffer> GetMainFrameBuffer() const override;
  std::shared_ptr<FrameBuffer> GetDisplayFrameBuffer() const override;

 private:
  // ---- 私有方法 ----
  void CreateDefaultFrameBuffer();
  void SwapFrameBuffers();

  // ---- 成员变量 ----
  std::shared_ptr<FrameBuffer> m_MainFrameBuffer;
  std::shared_ptr<FrameBuffer> m_DisplayFrameBuffer;
  bool m_IsRenderingScene = false;

  // ---- 新增Pipeline特有成员 ----
  std::unique_ptr<RenderContext> m_Context;
};

}  // namespace mite

#endif
