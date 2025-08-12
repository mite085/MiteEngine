#include "render_command.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后
#include "render_device.h"

namespace mite {
RenderCommand &RenderCommand::Get()
{
  static RenderCommand instance;
  return instance;
}

void RenderCommand::Init()
{
  // 默认渲染状态
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  LOG_INFO("RenderCommand initialized with default states");
}

void RenderCommand::Clear(const glm::vec4 &clearColor)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);
  instance.m_ClearColor = clearColor;
  instance.m_CommandQueue.push({CommandType::Clear, [] { /* 执行在Flush时处理 */ }});
}

void RenderCommand::Submit(const std::shared_ptr<OpenGLShader> &shader,
                           const std::shared_ptr<Mesh> &mesh,
                           const glm::mat4 &transform)
{
  if (!shader || !mesh) {
    LOG_WARN("RenderCommand::Submit - Null shader or vertex array");
    return;
  }

  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CommandQueue.push({CommandType::DrawIndexed, [=]() {
                                  shader->Bind();
                                  shader->SetMat4("u_Model", transform);
                                  IRenderDevice::Current().BindMesh(mesh);
                                  IRenderDevice::Current().DrawIndexed(
                                      mesh->GetIndexCount(), mesh->GetIndexOffset());
                                }});
}

void RenderCommand::SetViewport(int x, int y, int width, int height)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CommandQueue.push(
      {CommandType::SetViewport, [=]() { glViewport(x, y, width, height); }});
}

void RenderCommand::Flush()
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  // 优先执行非绘制命令
  while (!instance.m_CommandQueue.empty()) {
    const auto &cmd = instance.m_CommandQueue.front();

    switch (cmd.type) {
      case CommandType::Clear:
        glClearColor(instance.m_ClearColor.r,
                     instance.m_ClearColor.g,
                     instance.m_ClearColor.b,
                     instance.m_ClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;

      default:
        cmd.execute();  // 执行其他命令
        break;
    }

    instance.m_CommandQueue.pop();
  }
}
}  // namespace mite