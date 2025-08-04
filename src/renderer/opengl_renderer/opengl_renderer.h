#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "opengl_renderer/opegl_device.h"
#include "renderer.h"

#include "data/model.h"
#include "data/texture.h"
#include "renderable_entity.h"

namespace mite {
/**
 * OpenGL渲染器实现
 * 职责：
 * 1. 实现基类定义的渲染接口
 * 2. 管理OpenGL专属状态（如VAO、Shader Program）
 * 
 * TODO: 启用RenderCommand进行渲染
 */
class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer();
  ~OpenGLRenderer() override;
  void Initialize() override;

  // ---- 渲染指令 ----
  void BeginFrame() override;
  void EndFrame() override;

  /**
   * 渲染场景的核心接口
   * @param renderQueue 从SceneView获取的可渲染实体列表
   */
  void RenderScene(const std::vector<RenderableEntity> &renderQueue) override;

  // ---- 状态设置 ----
  void SetClearColor(const glm::vec4 &color) override;
  void SetViewport(uint32_t width, uint32_t height) override;

  // ---- 供Window调用的接口 ----
  intptr_t GetViewportFramebuffer() override;

 private:
  // ---- OpenGL专属状态 ----
  GLuint m_viewportFBO = 0;	// 默认帧缓冲（渲染到屏幕），OpenGL 规定其ID为0
};
}  // namespace mite

#endif
