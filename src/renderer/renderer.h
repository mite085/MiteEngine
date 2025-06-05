#ifndef MITE_RENDERER_API
#define MITE_RENDERER_API

#include "headers.h"
#include "glm/glm.hpp"
#include "window.h"
#include "scene_data.h"

namespace mite {

class VertexBuffer;
class IndexBuffer;
class VertexArray;
class Shader;

class Renderer {
 public:
  enum class API {
    None = 0,
    OpenGL = 1,
  };

  // 基本状态管理
  virtual bool Init(Window* window) = 0;
  virtual bool ShutDown() = 0;

  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
  virtual void SetDepthTesting(bool enabled) = 0;
  //virtual void SetBlendFunction(BlendFunc src, BlendFunc dst) = 0;
  //virtual void SetFaceCulling(bool enabled, CullFace face = CullFace::Back) = 0;
  //virtual void SetWireframeMode(bool enabled) = 0;

  // 清除操作
  virtual void SetClearColor(const glm::vec4 &color) = 0;
  virtual void Clear() = 0;
  virtual void DrawIndexed(VertexArray *vertexArray, uint32_t indexCount = 0) = 0;

  virtual VertexBuffer *CreateVertexBuffer(float *vertices, uint32_t size) = 0;
  virtual IndexBuffer *CreateIndexBuffer(uint32_t *indices, uint32_t count) = 0;
  virtual Shader *CreateShader(const std::string &vsPath, const std::string &fsPath) = 0;

  // 接受Scene提供的渲染友好数据执行绘制
  // TODO: RenderScene可以考虑拆分成BeginScene、
  // RenderBatches、RenderSkybox、RenderDebug、
  // EndScene等多个函数，由调用方控制渲染顺序
  virtual void RenderScene(const RenderData &render_data) = 0;

  // 交换缓冲
  virtual void SwapBuffers() = 0;
};

class VertexBuffer {
 public:
  virtual ~VertexBuffer() = default;

  virtual void Bind() = 0;
  virtual void Unbind() = 0;

  static VertexBuffer *Create(float *vertices, uint32_t size);
};

class IndexBuffer {
 public:
  virtual ~IndexBuffer() = default;

  virtual void Bind() = 0;
  virtual void Unbind() = 0;

  virtual uint32_t GetCount() const = 0;

  static IndexBuffer *Create(uint32_t *indices, uint32_t count);
};

class VertexArray {
 public:
  virtual ~VertexArray() = default;

  virtual void AddVertexBuffer(VertexBuffer *vb) = 0;
  virtual void SetIndexBuffer(IndexBuffer *ib) = 0;
  virtual void Bind() = 0;

  static VertexArray *Create();
};

class Shader {
 public:
  virtual ~Shader() = default;

  virtual void Bind() = 0;

  template<typename T> void SetUniform(const std::string &name, const T &value)
  {
    SetUniformImpl(name, value);
  }

 protected:
  virtual void SetUniformImpl(const std::string &name, const int &value) = 0;
  virtual void SetUniformImpl(const std::string &name, const float &value) = 0;
  virtual void SetUniformImpl(const std::string &name, const glm::vec3 &value) = 0;
  virtual void SetUniformImpl(const std::string &name, const glm::vec4 &value) = 0;
  virtual void SetUniformImpl(const std::string &name, const glm::mat4 &value) = 0;

  static Shader *Create(const std::string &vsPath, const std::string &fsPath);
};

}  // namespace mite

#endif