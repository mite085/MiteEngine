#include "opengl_command.h"
#include "basic_shader/shader_binding_point_manager.h"
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
                       [] {},  // 实际清除操作在Flush时执行
                       "Clear"});
}

void OpenGLRenderCommand::BindFrameBuffer(const std::shared_ptr<FrameBuffer> &framebuffer)
{
  if (!framebuffer) {
    m_Logger->warn("Attempt to bind null framebuffer");
    return;
  }

  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push(
      {CommandType::BindFrameBuffer, [framebuffer] { framebuffer->Bind(); }, "BindFrameBuffer"});
}

void OpenGLRenderCommand::UnbindFrameBuffer()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push({CommandType::UnbindFrameBuffer,
                       [] { glBindFramebuffer(GL_FRAMEBUFFER, 0); },
                       "UnbindFrameBuffer"});
}

void OpenGLRenderCommand::SetViewport(int x, int y, int width, int height)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  m_CommandQueue.push(
      {CommandType::SetViewport, [=]() { glViewport(x, y, width, height); }, "SetViewport"});
}

void OpenGLRenderCommand::SetRenderState(const std::shared_ptr<RenderState> &state)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  // 尝试转换为OpenGLRenderState
  auto glStatePtr = std::static_pointer_cast<OpenGLRenderState>(state);
  if (!glStatePtr) {
    LOG_ERROR("SetRenderState failed: cannot convert RenderState to OpenGLRenderState");
    return;
  }
  // 直接使用转换后的OpenGL状态
  m_CurrentGLState = *glStatePtr;
  m_CommandQueue.push({CommandType::SetRenderState,
                       [this, glState = *glStatePtr] { ApplyOpenGLState(glState); },
                       "SetRenderState"});
}

void OpenGLRenderCommand::BindCameraUBO(std::shared_ptr<CameraInstance> instance)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push(
      {CommandType::BindCameraUBO, [=]() { instance->BindUBO(); }, "Bind Camera UBO"});
}

void OpenGLRenderCommand::BindMaterialUBO(std::shared_ptr<MaterialInstance> instance)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  std::function<void(ExternalTextureType, TextureGPUHandle, TextureTarget)> bindExtTextureFunc =
      [=](ExternalTextureType type, TextureGPUHandle textureHandle, TextureTarget target) {
        m_Device->BindExternalTexture(type, textureHandle, target);
      };
  m_CommandQueue.push({CommandType::BindMaterialUBO,
                       [=]() { instance->Apply(bindExtTextureFunc); },
                       "Bind Material UBO and Textures"});
}

void OpenGLRenderCommand::BindLightSSBO(std::shared_ptr<LightShaderStorgeBuffer> instance)
{
  m_CommandQueue.push(
      {CommandType::BindLightSSBO, [=]() { instance->Bind(); }, "Bind Material UBO and Textures"});
}

void OpenGLRenderCommand::BindShader(
    std::shared_ptr<OpenGLShader> shader,
    std::function<void(std::shared_ptr<OpenGLShader>)> uniformSetup)
{
  if (!shader) {
    m_Logger->warn("Attempt to bind null shader");
    return;
  }
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::BindShader,
                       [this, shader, uniformSetup] {
                         // 绑定着色器
                         shader->Bind();

                         // 执行Uniform设置回调
                         if (uniformSetup) {
                           uniformSetup(shader);
                         }
                       },
                       "BindShader"});
}
void OpenGLRenderCommand::UnbindShader(std::shared_ptr<OpenGLShader> shader)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::UnbindShader,
                       [shader] {
                         if (shader)
                           shader->Unbind();
                       },
                       "UnbindShader"});
}
void OpenGLRenderCommand::BindRuntimeTexture(RuntimeTextureType type,
                                             TextureGPUHandle textureHandle,
                                             TextureTarget target)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::BindTextures,
                       [this, type, textureHandle, target] {
                         m_Device->BindRuntimeTexture(type, textureHandle, target);
                       },
                       "BindRuntimeTexture"});
}
void OpenGLRenderCommand::BindExternalTexture(ExternalTextureType type,
                                              TextureGPUHandle textureHandle,
                                              TextureTarget target)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::BindTextures,
                       [this, type, textureHandle, target] {
                         m_Device->BindExternalTexture(type, textureHandle, target);
                       },
                       "BindExternalTexture"});
}

void OpenGLRenderCommand::BindMesh(std::shared_ptr<Mesh> mesh)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push(
      {CommandType::BindMesh, [this, mesh] { m_Device->BindMesh(mesh); }, "BindMesh"});
}
void OpenGLRenderCommand::DrawMesh(uint32_t indexCount,
                                   uint32_t indexOffset,
                                   uint32_t primitiveType,
                                   uint32_t indexType)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::DrawMesh,
                       [this, indexCount, indexOffset, primitiveType, indexType] {
                         m_Device->DrawIndexed(indexCount, indexOffset, primitiveType, indexType);
                       },
                       "DrawMesh: count=" + std::to_string(indexCount)});
}

void OpenGLRenderCommand::DrawFullScreenQuad()
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::DrawIndexed,
                       [=] { m_Device->DrawFullScreenQuad(); },
                       "Submit Full Screen Quad Draw Call"});
}

void OpenGLRenderCommand::SubmitDrawCall(std::shared_ptr<MeshInstance> meshInstance,
                                         std::shared_ptr<OpenGLShader> shader)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::DrawIndexed,
                       [=] {
                         // 0. 绑定Model矩阵UBO
                         meshInstance->BindUBO();

                         // 1. 绑定网格VAO
                         m_Device->BindMesh(meshInstance->GetMesh());

                         // 2. 计算LOD
                         auto lodLevel = meshInstance->GetMeshLodLevel();

                         // 3. 绘制网格
                         m_Device->DrawMeshLOD(meshInstance->GetMesh(), lodLevel);
                       },
                       "Submit Mesh Draw Call"});
}

void OpenGLRenderCommand::PublishEventRuntimeTextureFinished(RuntimeTexturePtr texture,
                                                             std::string identify)
{
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  m_CommandQueue.push({CommandType::UnbindShader,
                       [texture, identify] {
                         EventBus::Publish<RuntimeTextureFinishedEvent>(
                             RuntimeTextureFinishedEvent(texture, identify));
                       },
                       "Publish Event Runtime Texture Finished: " + identify});
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

        default: {
          cmd.execute();
          break;
        }
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
                       [this, state] { ApplyOpenGLState(state); },
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
}  // namespace mite