#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "renderer.h"
#include "glad.h"
#include "GLFW/glfw3.h"// 必须在GLAD加载库之后
#include "opengl_renderer/opegl_device.h"

namespace mite {

class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer(OpenGLDevice &device);
  bool Init() override;
  void ShutDown() override;

  void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
  void SetDepthTesting(bool enabled) override;
  void SetClearColor(const glm::vec4 &color) override;
  void Clear() override;
  void DrawIndexed(VertexArray *vertexArray, uint32_t indexCount = 0) override;

  VertexBuffer *CreateVertexBuffer(float *vertices, uint32_t size) override;
  IndexBuffer *CreateIndexBuffer(uint32_t *indices, uint32_t count) override;
  ShaderBuffer *CreateShader(const std::string &vsPath, const std::string &fsPath) override;

  void SwapBuffers() override;

  //void RenderScene(const RenderData &render_data) override;

  
  OpenGLDevice &GetRenderDevice()
  {
    return m_Device;
  }

 protected:
  OpenGLDevice &m_Device;
};


}  // namespace mite

#endif
