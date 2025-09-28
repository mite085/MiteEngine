#include "render_command.h"
#include "render_device.h"

namespace mite {
RenderCommand &RenderCommand::Get()
{
  static RenderCommand instance;
  return instance;
}

void RenderCommand::Init()
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  // 创建日志系统
  instance.m_Logger = LoggerSystem::CreateModuleLogger("Mite OpenGL Render Command");
  instance.m_Logger->info("OpenGL Renderer Command created");

  // 设置默认渲染状态
  instance.m_CurrentState = {
      true,                    // depthTest
      GL_LESS,                 // depthFunc
      true,                    // blend
      GL_SRC_ALPHA,            // blendSrc
      GL_ONE_MINUS_SRC_ALPHA,  // blendDst
      true,                    // cullFace
      GL_BACK                  // cullFaceMode
  };

  // 提交初始化命令（确保在渲染线程执行）
  instance.m_CommandQueue.push({CommandType::SetRenderState,
                                [] {
                                  glEnable(GL_DEPTH_TEST);  // 深度测试
                                  glDepthFunc(GL_LESS);
                                  glEnable(GL_BLEND);
                                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                  glEnable(GL_CULL_FACE);  // 面剔除
                                  glCullFace(GL_BACK);     // 剔除背面
                                  glFrontFace(GL_CCW);     // 逆时针为正面
                                },
                                "InitRenderState"});

  instance.m_Logger->info("RenderCommand initialized with default states");
}

void RenderCommand::Clear(uint32_t clearFlags,
                          const glm::vec4 &clearColor,
                          float depthClear,
                          int stencilClear)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  // 更新清除参数
  instance.m_ClearColor = clearColor;
  instance.m_DepthClearValue = depthClear;
  instance.m_StencilClearValue = stencilClear;
  instance.m_ClearFlags = clearFlags;

  // 提交清除命令
  instance.m_CommandQueue.push({CommandType::Clear,
                                [] {},  // 实际清除操作在Flush时执行
                                "Clear"});
}

void RenderCommand::BindFrameBuffer(const FrameBuffer::Ptr &framebuffer)
{
  auto &instance = Get();

  if (!framebuffer) {
    instance.m_Logger->warn("Attempt to bind null framebuffer");
    return;
  }

  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CommandQueue.push(
      {CommandType::BindFrameBuffer, [framebuffer] { framebuffer->Bind(); }, "BindFrameBuffer"});
}

void RenderCommand::UnbindFrameBuffer()
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CommandQueue.push({CommandType::UnbindFrameBuffer,
                                [] { glBindFramebuffer(GL_FRAMEBUFFER, 0); },
                                "UnbindFrameBuffer"});
}

void RenderCommand::Submit(RenderableItem item, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  std::function<void(TextureGPUHandle, uint32_t)> bindTextureFunc = [](TextureGPUHandle handle,
                                                                       uint32_t slot) {
    IRenderDevice::Current().BindTexture(handle, slot);
  };

  instance.m_CommandQueue.push(
      {CommandType::DrawIndexed,
       [=]() {
         // 1. 应用材质（绑定着色器、上传uniforms、绑定纹理）
         if (!item.material) {
           LOG_ERROR("Invalid Material Instance");
           return;
         }
         item.material->Apply(bindTextureFunc);

         // 2. 设置模型矩阵（从世界变换获取）
         auto shader = item.material->GetShader();
         if (shader) {
           shader->SetMat4("u_Model", item.worldTransform);
           shader->SetMat4("u_View", viewMatrix);
           shader->SetMat4("u_Projection", projectionMatrix);
         }

         // 3. 绑定网格VAO
         IRenderDevice::Current().BindMesh(item.mesh);

         // 4. 绘制网格:
         // TODO:
         // 深度测试或模板测试冲突，如果此时启用了深度测试，由于深度附件未正确初始化，会导致绘制被丢弃。
         IRenderDevice::Current().DrawIndexed(item.mesh.GetIndexCount(),
                                              item.mesh.GetIndexOffset(),
                                              GL_TRIANGLES,
                                              GL_UNSIGNED_INT,
                                              false);
       },
       "DrawIndexed mesh from model: " + item.mesh.GetModelHandle().path});
}

void RenderCommand::SetViewport(int x, int y, int width, int height)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CommandQueue.push(
      {CommandType::SetViewport, [=]() { glViewport(x, y, width, height); }, "SetViewport"});
}

void RenderCommand::SetRenderState(const RenderState &state)
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  instance.m_CurrentState = state;

  instance.m_CommandQueue.push({CommandType::SetRenderState,
                                [state] {
                                  // 深度测试设置
                                  if (state.depthTest) {
                                    glEnable(GL_DEPTH_TEST);
                                    glDepthFunc(state.depthFunc);
                                  }
                                  else {
                                    glDisable(GL_DEPTH_TEST);
                                  }

                                  // 混合设置
                                  if (state.blend) {
                                    glEnable(GL_BLEND);
                                    glBlendFunc(state.blendSrc, state.blendDst);
                                  }
                                  else {
                                    glDisable(GL_BLEND);
                                  }

                                  // 面剔除设置
                                  if (state.cullFace) {
                                    glEnable(GL_CULL_FACE);
                                    glCullFace(state.cullFaceMode);
                                  }
                                  else {
                                    glDisable(GL_CULL_FACE);
                                  }
                                },
                                "SetRenderState"});
}

void RenderCommand::Flush()
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  while (!instance.m_CommandQueue.empty()) {
    const auto &cmd = instance.m_CommandQueue.front();

    try {
      switch (cmd.type) {
        case CommandType::Clear:
          glClearColor(instance.m_ClearColor.r,
                       instance.m_ClearColor.g,
                       instance.m_ClearColor.b,
                       instance.m_ClearColor.a);
          glClearDepth(instance.m_DepthClearValue);
          glClearStencil(instance.m_StencilClearValue);
          glClear(instance.m_ClearFlags);
          break;

        default:
          cmd.execute();
          break;
      }
    }
    catch (const std::exception &e) {
      instance.m_Logger->error("Failed to execute command {}: {}", cmd.debugName, e.what());
    }

    instance.m_CommandQueue.pop();
  }
}

void RenderCommand::ClearQueue()
{
  auto &instance = Get();
  std::lock_guard<std::mutex> lock(instance.m_QueueMutex);

  while (!instance.m_CommandQueue.empty()) {
    instance.m_CommandQueue.pop();
  }
}
}  // namespace mite