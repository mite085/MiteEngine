#include "opengl_renderer.h"

namespace mite {
OpenGLRenderer::OpenGLRenderer()
{
  // 创建日志系统
  m_Logger = LoggerSystem::CreateModuleLogger("Mite OpenGL Renderer");
  m_Logger->info("OpenGL Renderer created");
}

OpenGLRenderer::~OpenGLRenderer()
{
  m_Logger->info("OpenGL Renderer destroyed");
}

void OpenGLRenderer::Initialize()
{
  // 初始化默认FrameBuffer
  CreateDefaultFrameBuffer();

  // 通过RenderCommand初始化OpenGL状态
  RenderCommand::Init();

  m_Logger->info("OpenGL Renderer initialized");
}

void OpenGLRenderer::CreateDefaultFrameBuffer()
{
  // 创建FrameBuffer规格
  FrameBufferSpec spec;
  spec.attachments = {
      {FrameBufferAttachmentType::Color, GL_RGBA8},  // 颜色附件
      {FrameBufferAttachmentType::Depth}             // 深度附件
  };

  // 创建两个相同的FrameBuffer用于双缓冲
  m_MainFrameBuffer = std::make_shared<FrameBuffer>(spec);
  m_DisplayFrameBuffer = std::make_shared<FrameBuffer>(spec);

  if (!m_MainFrameBuffer->IsComplete() || !m_DisplayFrameBuffer->IsComplete()) {
    m_Logger->error("Failed to create complete framebuffers for double buffering");
    throw std::runtime_error("Framebuffers are incomplete");
  }

  m_Logger->info("Created default framebuffer");
}

void OpenGLRenderer::BeginFrame()
{
  // 重置渲染状态
  m_IsRenderingScene = true;

  // 绑定主渲染FrameBuffer
  RenderCommand::BindFrameBuffer(m_MainFrameBuffer);

  // 本次绘制首次绑定FrameBuffer，提交FrameBuffer的清屏命令
  RenderCommand::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, m_ClearColor);

  // 根据MainBuffer设置视口大小
  auto size = m_MainFrameBuffer->GetSize();
  RenderCommand::SetViewport(0, 0, size.x, size.y);

  //m_Logger->debug("BeginFrame: Submitted commands for scene rendering");
}

void OpenGLRenderer::EndFrame()
{
  // 结束场景渲染阶段
  m_IsRenderingScene = false;

  // 提交解绑FrameBuffer命令
  RenderCommand::UnbindFrameBuffer();

  // 重置OpenGL状态
  RenderCommand::PushCustomCommand(
      [] {
        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
      },
      "ResetGLState");

  // 执行所有命令
  RenderCommand::Flush();

  // 交换双缓冲
  SwapFrameBuffers();

  //m_Logger->debug("EndFrame: Executed all commands and swapped buffers");
}

void OpenGLRenderer::RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                                 const glm::mat4 viewMatrix,
                                 const glm::mat4 projectionMatrix)
{
  if (!renderQueue) {
    m_Logger->warn("OpenGLRenderer::Render called with null renderQueue");
    return;
  }

  // 检查是否在正确的渲染阶段
  if (!m_IsRenderingScene) {
    m_Logger->warn("RenderScene called outside of scene rendering phase");
    return;
  }

  // 1. 按队列类型顺序渲染（通常：不透明 -> Alpha测试 -> 透明）
  const std::vector<RenderQueue::QueueType> renderOrder = {RenderQueue::QueueType::Opaque,
                                                           RenderQueue::QueueType::AlphaTest,
                                                           RenderQueue::QueueType::Transparent,
                                                           RenderQueue::QueueType::Custom};

  // 2. 遍历所有队列类型
  for (auto queueType : renderOrder) {
    const auto &items = renderQueue->GetItems(queueType);
    if (items.empty())
      continue;

    // 3. 渲染当前队列的所有项
    for (const auto &item : items) {
      if (MaterialSystem::GetInstance(item.material)->GetShader()->GetHandle().programId == 0) {
        m_Logger->warn("Invalid renderable item - missing material");
        continue;
      }

      // 4. 通过RenderCommand提交绘制命令
      RenderCommand::Submit(item, viewMatrix, projectionMatrix);
    }
  }

  //m_Logger->debug("RenderScene: Submitted scene rendering commands");
}

void OpenGLRenderer::SetClearColor(const glm::vec4 &color)
{
  m_ClearColor = color;
}

std::shared_ptr<FrameBuffer> OpenGLRenderer::GetMainFrameBuffer() const
{
  return m_MainFrameBuffer;
}

std::shared_ptr<FrameBuffer> OpenGLRenderer::GetDisplayFrameBuffer() const
{
  return m_DisplayFrameBuffer;
}

void OpenGLRenderer::SwapFrameBuffers()
{
  // 简单的指针交换（双缓冲）
  std::swap(m_MainFrameBuffer, m_DisplayFrameBuffer);

  //m_Logger->debug("SwapFrameBuffers: Swapped buffers for UI display");
}

}  // namespace mite