#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "opengl_renderer/opegl_device.h"
#include "renderer.h"

#include "basic_data/model.h"
#include "basic_data/texture.h"
#include "renderable_item.h"

namespace mite {
/**
 * OpenGL渲染器实现类
 * 职责：
 * 1. 实现基类定义的渲染接口
 * 2. 管理OpenGL专属状态（如VAO、Shader Program）
 * 3. 集成FrameBuffer系统
 */
class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer();
  ~OpenGLRenderer() override;

  // ---- 初始化 ----
  void Initialize() override;

  // ---- 帧控制 ----
  void BeginFrame() override;
  void EndFrame() override;

  // ---- 场景渲染 ----
  void RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                   const glm::mat4 viewMatrix,
                   const glm::mat4 projectionMatrix) override;

  // ---- 状态设置 ----
  void SetClearColor(const glm::vec4 &color) override;
  void SetViewport(uint32_t width, uint32_t height) override;

  // ---- UI接口 ----
  std::shared_ptr<FrameBuffer> GetViewportFrameBuffer() const override;
  intptr_t GetViewportFramebufferID() const override;

 private:
  // ---- 私有方法 ----
  /**
   * @brief 创建默认FrameBuffer
   */
  void CreateDefaultFrameBuffer();

  // ---- 成员变量 ----
  std::shared_ptr<FrameBuffer> m_ViewportFrameBuffer;  // 视口FrameBuffer
  Logger m_Logger;                                     // 日志系统
};
}  // namespace mite

#endif
