#include "opengl_pipeline.h"
#include "render_stages/forward_stage.h"

namespace mite {

OpenGLPipeline::OpenGLPipeline() : RenderPipeline()
{
  m_Logger->info("OpenGL Pipeline created");
}

OpenGLPipeline::~OpenGLPipeline()
{
  m_Logger->info("OpenGL Pipeline destroyed");
}

void OpenGLPipeline::Initialize()
{
  // 创建默认FrameBuffer
  CreateDefaultFrameBuffer();

  // 初始化RenderCommand
  RenderCommand::Get().Init();

  // 创建渲染上下文
  m_Context = std::make_unique<RenderContext>();

  // 添加默认阶段
  AddStage(std::make_unique<ForwardStage>());

  // 初始化所有阶段
  for (auto &stage : m_Stages) {
    if (stage->IsEnabled()) {
      stage->Initialize();
    }
  }

  m_Logger->info("OpenGL Pipeline initialized");
}

void OpenGLPipeline::Shutdown()
{
  // 关闭所有阶段
  for (auto &stage : m_Stages) {
    stage->Shutdown();
  }

  m_Logger->info("OpenGL Pipeline shutdown");
}

void OpenGLPipeline::BeginFrame()
{
  // 重置渲染状态
  m_IsRenderingScene = true;

  // 绑定主渲染FrameBuffer
  RenderCommand::Get().BindFrameBuffer(m_MainFrameBuffer);

  // 清屏命令
  RenderCommand::Get().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, m_ClearColor);

  // 设置视口
  auto size = m_MainFrameBuffer->GetSize();
  RenderCommand::Get().SetViewport(0, 0, size.x, size.y);

  //m_Logger->debug("Pipeline BeginFrame completed");
}

void OpenGLPipeline::EndFrame()
{
  // 结束场景渲染阶段
  m_IsRenderingScene = false;

  // 解绑FrameBuffer
  RenderCommand::Get().UnbindFrameBuffer();

  // 重置OpenGL状态
  RenderCommand::Get().PushCustomCommand(
      [] {
        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
      },
      "ResetGLState");

  // 执行所有命令
  RenderCommand::Get().Flush();

  // 交换双缓冲
  SwapFrameBuffers();

  //m_Logger->debug("Pipeline EndFrame completed");
}

void OpenGLPipeline::RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                                 const glm::mat4 viewMatrix,
                                 const glm::mat4 projectionMatrix)
{
  if (!renderQueue) {
    m_Logger->warn("RenderScene called with null renderQueue");
    return;
  }

  // 检查是否在正确的渲染阶段
  if (!m_IsRenderingScene) {
    m_Logger->warn("RenderScene called outside of scene rendering phase");
    return;
  }

  // 设置上下文数据
  m_Context->SetSceneData(renderQueue, viewMatrix, projectionMatrix);
  m_Context->SetFrameBuffer(m_MainFrameBuffer);

  // 执行所有启用的阶段（新增Pipeline逻辑）
  for (auto &stage : m_Stages) {
    if (stage->IsEnabled()) {
      stage->Execute(*m_Context);
    }
  }

  //m_Logger->debug("Pipeline RenderScene completed");
}

void OpenGLPipeline::SetClearColor(const glm::vec4 &color)
{
  m_ClearColor = color;
}

std::shared_ptr<FrameBuffer> OpenGLPipeline::GetMainFrameBuffer() const
{
  return m_MainFrameBuffer;
}

std::shared_ptr<FrameBuffer> OpenGLPipeline::GetDisplayFrameBuffer() const
{
  return m_DisplayFrameBuffer;
}

void OpenGLPipeline::CreateDefaultFrameBuffer()
{
  // 创建FrameBuffer规格
  FrameBufferSpec spec;
  spec.attachments = {{FrameBufferAttachmentType::Color, GL_RGBA8},  // 颜色附件
                      {FrameBufferAttachmentType::Depth}};           // 深度附件

  // 创建两个相同的FrameBuffer用于双缓冲
  m_MainFrameBuffer = std::make_shared<FrameBuffer>(spec);
  m_DisplayFrameBuffer = std::make_shared<FrameBuffer>(spec);

  if (!m_MainFrameBuffer->IsComplete() || !m_DisplayFrameBuffer->IsComplete()) {
    m_Logger->error("Failed to create complete framebuffers for double buffering");
    throw std::runtime_error("Framebuffers are incomplete");
  }

  m_Logger->info("Created default framebuffer");
}

void OpenGLPipeline::SwapFrameBuffers()
{
  // 完全接管原OpenGLRenderer的实现
  std::swap(m_MainFrameBuffer, m_DisplayFrameBuffer);
  //m_Logger->debug("Pipeline swapped buffers");
}

}  // namespace mite
