#include "opengl_renderer.h"

namespace mite {

// GLFW报错回调函数
static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool OpenGLRenderer::Init(Window *window)
{
  // 启用自定义错误回调函数
  glfwSetErrorCallback(glfw_error_callback);

  // 初始化glfw
  if (!glfwInit())
    return false;

  // 配置glfw
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  // 启用 OpenGL Core Profile，而不是 Compatibility Profile（兼容模式）
  // 注意：仅OpenGL3.2以上版本支持Core Profile
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // macOS 只支持 Core Profile，且必须显式启用 Forward Compatibility（向前兼容）
#ifndef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif  // !__APPLE__

  return true;
}

bool OpenGLRenderer::ShutDown()
{
  return false;
}

void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

void OpenGLRenderer::SetDepthTesting(bool enabled) {}

void OpenGLRenderer::SetClearColor(const glm::vec4 &color) {}

void OpenGLRenderer::Clear() {}

void OpenGLRenderer::DrawIndexed(VertexArray *vertexArray, uint32_t indexCount) {}

VertexBuffer *OpenGLRenderer::CreateVertexBuffer(float *vertices, uint32_t size)
{
  return nullptr;
}

IndexBuffer *OpenGLRenderer::CreateIndexBuffer(uint32_t *indices, uint32_t count)
{
  return nullptr;
}

Shader *OpenGLRenderer::CreateShader(const std::string &vsPath, const std::string &fsPath)
{
  return nullptr;
}
void OpenGLRenderer::SwapBuffers() {}

void OpenGLRenderer::RenderScene(const RenderData &render_data) {
  //// TODO: 设置相机UBO
  //SetCameraUniforms(renderData.GetCameraData());

  //// 渲染不透明物体
  //for (const auto &batch : renderData.GetBatches(RenderPass::Opaque)) {
  //  BindMaterial(batch.material);
  //  for (const auto &item : batch.items) {
  //    SetObjectUniforms(item.transform);
  //    DrawMesh(item.mesh, item.submeshIndex);
  //  }
  //}

  //// 渲染透明物体（已排序）
  //for (const auto &batch : renderData.GetBatches(RenderPass::Transparent)) {
  //  BindMaterial(batch.material);
  //  for (const auto &item : batch.items) {
  //    SetObjectUniforms(item.transform);
  //    DrawMesh(item.mesh, item.submeshIndex);
  //  }
  //}
}
}  // namespace mite