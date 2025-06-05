#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "headers.h"

#include "renderer.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"// 必须在GLAD加载库之后

namespace mite {

class OpenGLRenderer : public Renderer {
 public:
  bool Init(Window *window) override;
  bool ShutDown() override;

  void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
  void SetDepthTesting(bool enabled) override;
  void SetClearColor(const glm::vec4 &color) override;
  void Clear() override;
  void DrawIndexed(VertexArray *vertexArray, uint32_t indexCount = 0) override;

  VertexBuffer *CreateVertexBuffer(float *vertices, uint32_t size) override;
  IndexBuffer *CreateIndexBuffer(uint32_t *indices, uint32_t count) override;
  Shader *CreateShader(const std::string &vsPath, const std::string &fsPath) override;

  void SwapBuffers() override;

  void RenderScene(const RenderData &render_data) override;
};


}  // namespace mite

#endif
