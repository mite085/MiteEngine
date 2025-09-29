#include "opengl_command.h"
#include "opengl_device.h"

namespace mite {
void OpenGLRenderCommand::Init()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  // 创建日志系统
  m_Logger = LoggerSystem::CreateModuleLogger("Mite OpenGL Render Command");
  m_Logger->info("OpenGL Renderer Command created");

  // 创建Device
  m_Device = std::make_unique<OpenGLDevice>();

  // 设置默认渲染状态（全部使用默认值）
  m_CurrentGLState = OpenGLRenderState{};

  // 提交初始化命令（确保在渲染线程执行）
  m_CommandQueue.push({CommandType::SetRenderState,
                       [this] {
                         ApplyOpenGLState(m_CurrentGLState);
                         // 设置默认的正面朝向
                         glFrontFace(GL_CCW);
                         CheckGLError();
                       },
                       "InitRenderState"});

  m_Logger->info("RenderCommand initialized with default states");
}

void OpenGLRenderCommand::Clear(uint32_t clearFlags,
                                const glm::vec4 &clearColor,
                                float depthClear,
                                int stencilClear)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  // 更新清除参数
  m_ClearColor = clearColor;
  m_DepthClearValue = depthClear;
  m_StencilClearValue = stencilClear;
  m_ClearFlags = clearFlags;

  // 提交清除命令
  m_CommandQueue.push({CommandType::Clear,
                       [] { CheckGLError(); },  // 实际清除操作在Flush时执行
                       "Clear"});
}

void OpenGLRenderCommand::BindFrameBuffer(const FrameBuffer::Ptr &framebuffer)
{
  if (!framebuffer) {
    m_Logger->warn("Attempt to bind null framebuffer");
    return;
  }

  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push({CommandType::BindFrameBuffer,
                       [framebuffer] {
                         framebuffer->Bind();
                         CheckGLError();
                       },
                       "BindFrameBuffer"});
}

void OpenGLRenderCommand::UnbindFrameBuffer()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push({CommandType::UnbindFrameBuffer,
                       [] {
                         glBindFramebuffer(GL_FRAMEBUFFER, 0);
                         CheckGLError();
                       },
                       "UnbindFrameBuffer"});
}

void OpenGLRenderCommand::Submit(RenderableItem item,
                                 glm::mat4 viewMatrix,
                                 glm::mat4 projectionMatrix)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  std::function<void(TextureGPUHandle, uint32_t)> bindTextureFunc =
      [=](TextureGPUHandle handle, uint32_t slot) { m_Device->BindTexture(handle, slot); };

  m_CommandQueue.push({CommandType::DrawIndexed,
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
                         m_Device->BindMesh(item.mesh);

                         // 4. 绘制网格:
                         m_Device->DrawIndexed(item.mesh.GetIndexCount(),
                                               item.mesh.GetIndexOffset(),
                                               GL_TRIANGLES,
                                               GL_UNSIGNED_INT);
                         CheckGLError();
                       },
                       "DrawIndexed mesh from model: " + item.mesh.GetModelHandle().path});
}

void OpenGLRenderCommand::SetViewport(int x, int y, int width, int height)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push({CommandType::SetViewport,
                       [=]() {
                         glViewport(x, y, width, height);
                         CheckGLError();
                       },
                       "SetViewport"});
}

void OpenGLRenderCommand::SetRenderState(const RenderState &state)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  // 将基础状态转换为OpenGL特定状态
  OpenGLRenderState glState;
  glState.depthTest = state.depthTest;
  glState.blend = state.blend;
  glState.cullFace = state.cullFace;

  // 使用OpenGL默认值填充平台特定字段
  glState.depthFunc = GL_LESS;
  glState.blendSrc = GL_SRC_ALPHA;
  glState.blendDst = GL_ONE_MINUS_SRC_ALPHA;
  glState.cullFaceMode = GL_BACK;
  m_CurrentGLState = glState;
  m_CommandQueue.push({CommandType::SetRenderState,
                       [this, glState] {
                         ApplyOpenGLState(glState);
                         CheckGLError();
                       },
                       "SetRenderState"});
}

void OpenGLRenderCommand::Flush()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  while (!m_CommandQueue.empty()) {
    const auto &cmd = m_CommandQueue.front();

    try {
      switch (cmd.type) {
        case CommandType::Clear:
          glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
          glClearDepth(m_DepthClearValue);
          glClearStencil(m_StencilClearValue);
          glClear(m_ClearFlags);
          break;

        default:
          cmd.execute();
          break;
      }
    }
    catch (const std::exception &e) {
      m_Logger->error("Failed to execute command {}: {}", cmd.debugName, e.what());
    }

    m_CommandQueue.pop();
  }
}

void OpenGLRenderCommand::ClearQueue()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  while (!m_CommandQueue.empty()) {
    m_CommandQueue.pop();
  }
}

void OpenGLRenderCommand::SetRenderState(const OpenGLRenderState &state)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CurrentGLState = state;
  m_CommandQueue.push({CommandType::SetRenderState,
                       [this, state] {
                         ApplyOpenGLState(state);
                         CheckGLError();
                       },
                       "SetRenderState(GL)"});
}

void OpenGLRenderCommand::ApplyOpenGLState(const OpenGLRenderState &state)
{
  // 深度测试设置
  if (state.depthTest) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(state.depthFunc);
  }
  else {
    glDisable(GL_DEPTH_TEST);
  }

  // 深度写入控制（新增）
  glDepthMask(state.depthWrite ? GL_TRUE : GL_FALSE);

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

  // 颜色写入掩码
  glColorMask(state.colorWriteR ? GL_TRUE : GL_FALSE,
              state.colorWriteG ? GL_TRUE : GL_FALSE,
              state.colorWriteB ? GL_TRUE : GL_FALSE,
              state.colorWriteA ? GL_TRUE : GL_FALSE);

  // 多边形模式
  if (state.wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // 模板测试设置（基础支持，待后续扩展）
  if (state.stencilTest) {
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(state.stencilFunc, state.stencilRef, state.stencilMask);
    glStencilOp(state.stencilFail, state.stencilPassDepthFail, state.stencilPassDepthPass);
  }
  else {
    glDisable(GL_STENCIL_TEST);
  }
}
void OpenGLRenderCommand::CheckGLError()
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    const char *errorStr = "";
    switch (err) {
      case GL_INVALID_ENUM:
        errorStr = "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        errorStr = "GL_INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        errorStr = "GL_INVALID_OPERATION";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      case GL_OUT_OF_MEMORY:
        errorStr = "GL_OUT_OF_MEMORY";
        break;
      default:
        errorStr = "Unknown Error";
    }

    LOG_ERROR("OpenGL Error: {} ({})", err, errorStr);
  }
}
}  // namespace mite