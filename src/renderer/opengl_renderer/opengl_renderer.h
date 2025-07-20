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
 */
class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer();
  ~OpenGLRenderer() override;
  void Initialize() override;

  // ---- 渲染指令 ----
  void BeginFrame() override;
  void EndFrame() override;

  // TODO: Draw操作全权交付给渲染队列进行，此处不应当单独执行Draw方法。
  void DrawModel(const Model &model, const glm::mat4 &transform) override;

  /**
   * 渲染场景的核心接口
   * @param renderQueue 从SceneView获取的可渲染实体列表
   */
  void RenderScene(const std::vector<RenderableEntity> &renderQueue) override;

 private:
  // ---- OpenGL专属状态 ----
  GLuint defaultFBO_ = 0;
  GLuint currentShader_ = 0;
};
}  // namespace mite

#endif
