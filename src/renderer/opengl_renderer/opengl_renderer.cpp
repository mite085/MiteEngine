#include "opengl_renderer.h"

namespace mite {

// GLFW����ص�����
static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool OpenGLRenderer::Init(Window *window)
{
  // �����Զ������ص�����
  glfwSetErrorCallback(glfw_error_callback);

  // ��ʼ��glfw
  if (!glfwInit())
    return false;

  // ����glfw
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  // ���� OpenGL Core Profile�������� Compatibility Profile������ģʽ��
  // ע�⣺��OpenGL3.2���ϰ汾֧��Core Profile
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // macOS ֻ֧�� Core Profile���ұ�����ʽ���� Forward Compatibility����ǰ���ݣ�
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
  //// TODO: �������UBO
  //SetCameraUniforms(renderData.GetCameraData());

  //// ��Ⱦ��͸������
  //for (const auto &batch : renderData.GetBatches(RenderPass::Opaque)) {
  //  BindMaterial(batch.material);
  //  for (const auto &item : batch.items) {
  //    SetObjectUniforms(item.transform);
  //    DrawMesh(item.mesh, item.submeshIndex);
  //  }
  //}

  //// ��Ⱦ͸�����壨������
  //for (const auto &batch : renderData.GetBatches(RenderPass::Transparent)) {
  //  BindMaterial(batch.material);
  //  for (const auto &item : batch.items) {
  //    SetObjectUniforms(item.transform);
  //    DrawMesh(item.mesh, item.submeshIndex);
  //  }
  //}
}
}  // namespace mite