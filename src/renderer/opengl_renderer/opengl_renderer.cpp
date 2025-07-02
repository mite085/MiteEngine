#include "opengl_renderer.h"

namespace mite {
OpenGLRenderer::OpenGLRenderer(OpenGLDevice &device) : m_Device(device) {}

bool OpenGLRenderer::Init()
{
  // OpenGL的初始化由Window负责。而非renderer
  //
  // 1. 检查上下文是否已存在（通过 GLAD）
  if (!gladLoadGL()) {
    LOG_ERROR("Failed to initialize OpenGL loader (GLAD)");
    return false;
  }

  // 2. 设置默认渲染状态
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 3. 初始化内置资源（如默认着色器）
  // if (!LoadDefaultShader()) {
  //  return false;
  //}

  return true;
}

void OpenGLRenderer::ShutDown()
{
  // 1. 释放所有 GPU 资源
  // ReleaseDefaultShader();
  // ClearAllBuffers();  // 清理 VertexBuffer/IndexBuffer 等

  // 2. 重置 OpenGL 状态
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
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

ShaderBuffer *OpenGLRenderer::CreateShader(const std::string &vsPath, const std::string &fsPath)
{
  return nullptr;
}
void OpenGLRenderer::SwapBuffers() {}

// void OpenGLRenderer::RenderScene(const RenderData &render_data) {
//// TODO: 设置相机UBO
// SetCameraUniforms(renderData.GetCameraData());

//// 渲染不透明物体
// for (const auto &batch : renderData.GetBatches(RenderPass::Opaque)) {
//   BindMaterial(batch.material);
//   for (const auto &item : batch.items) {
//     SetObjectUniforms(item.transform);
//     DrawMesh(item.mesh, item.submeshIndex);
//   }
// }

//// 渲染透明物体（已排序）
// for (const auto &batch : renderData.GetBatches(RenderPass::Transparent)) {
//   BindMaterial(batch.material);
//   for (const auto &item : batch.items) {
//     SetObjectUniforms(item.transform);
//     DrawMesh(item.mesh, item.submeshIndex);
//   }
// }
//}
}  // namespace mite