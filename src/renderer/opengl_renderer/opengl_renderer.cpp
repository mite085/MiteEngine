#include "opengl_renderer.h"
#include "asset_manager.h"

namespace mite {
OpenGLRenderer::OpenGLRenderer()
{  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite OpenGL Renderer");
  m_Logger->trace("Created OpenGL Renderer");
}

OpenGLRenderer::~OpenGLRenderer() {}

void OpenGLRenderer::Initialize()
{
  // 初始化OpenGL默认状态
  glEnable(GL_DEPTH_TEST);  // 深度测试
  glEnable(GL_CULL_FACE);   // 面剔除
  glCullFace(GL_BACK);      // 剔除背面
  glFrontFace(GL_CCW);      // 逆时针为正面
}

// ===================== 渲染指令 =====================
void OpenGLRenderer::BeginFrame()
{
  SetClearColor(clearColor_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndFrame()
{
  // 确保所有命令提交
  glFlush();

  // 重置OpenGL状态机
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
  glActiveTexture(GL_TEXTURE0);

  // 注意：不包含交换缓冲区的操作，由窗口系统负责
}

void OpenGLRenderer::RenderScene(const std::shared_ptr<Camera> mainCamera,
                                 const std::vector<std::shared_ptr<RenderableItem>> &renderQueue)
{
  // 定义纹理绑定lambda函数
  auto bindTextureFunc = [](TextureGPUHandle handle, uint32_t slot) {
    IRenderDevice::Current().BindTexture(handle, slot);
  };

  // 获取视图和投影矩阵
  glm::mat4 viewMatrix = mainCamera->GetViewMatrix();
  glm::mat4 projectionMatrix = mainCamera->GetProjectionMatrix();

  // 遍历渲染队列
  for (const auto &item : renderQueue) {  
    // 0. 检查渲染实体是否有效
    if (!item->materialInstance || item->mesh->GetModelHandle()->vertexArray == 0) {
      m_Logger->warn("Invalid renderable item - missing material or mesh");
      continue;
    }

    // 1. 应用材质（绑定着色器、上传uniforms、绑定纹理）
    item->materialInstance->Apply(bindTextureFunc);

    // 2. 设置模型矩阵（从世界变换获取）
    auto shader = item->materialInstance->GetShader();
    if (shader) {
      shader->SetMat4("u_Model", item->worldTransform);
      shader->SetMat4("u_View", viewMatrix);
      shader->SetMat4("u_Projection", projectionMatrix);
    }

    // 3. 绑定网格VAO
    IRenderDevice::Current().BindMesh(item->mesh);

    // 4. 绘制网格:
    IRenderDevice::Current().DrawIndexed(item->mesh->GetIndexCount(),
                                         item->mesh->GetIndexOffset());

    // 5. 解绑（可选，减少状态切换）
    glBindVertexArray(0);
  }

  // 检查OpenGL错误
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    m_Logger->error("OpenGL error after rendering: {}", static_cast<int>(err));
  }
}

void OpenGLRenderer::SetClearColor(const glm::vec4 &color)
{
  glClearColor(color.r, color.g, color.b, color.a);
}
void OpenGLRenderer::SetViewport(uint32_t width, uint32_t height)
{
  glViewport(0, 0, viewportSize_.x, viewportSize_.y);
}

intptr_t OpenGLRenderer::GetViewportFramebuffer()
{
  return static_cast<intptr_t>(m_viewportFBO);
}

}  // namespace mite