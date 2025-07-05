#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后
#include "opengl_renderer/opegl_device.h"
#include "renderer.h"

#include "model.h"
#include "texture.h"

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
  void DrawModel(const Model &model, const glm::mat4 &transform) override;

 private:
  // ---- OpenGL专属状态 ----
  GLuint defaultFBO_ = 0;
  GLuint currentShader_ = 0;
};
}  // namespace mite

#endif
