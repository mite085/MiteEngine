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
  spec.width = m_ViewportSize.x;
  spec.height = m_ViewportSize.y;
  spec.attachments = {
      {FrameBufferAttachmentType::Color, GL_RGBA8},  // 颜色附件
      {FrameBufferAttachmentType::Depth}             // 深度附件
  };

  // 创建FrameBuffer
  m_ViewportFrameBuffer = std::make_shared<FrameBuffer>(spec);

  if (!m_ViewportFrameBuffer->IsComplete()) {
    m_Logger->error("Failed to create complete framebuffer");
    throw std::runtime_error("Framebuffer is incomplete");
  }

  m_Logger->info("Created default framebuffer ({}x{})", m_ViewportSize.x, m_ViewportSize.y);
}

void OpenGLRenderer::BeginFrame()
{
  // 通过RenderCommand提交清屏命令
  RenderCommand::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, m_ClearColor);

  // 绑定视口FrameBuffer
  RenderCommand::BindFrameBuffer(m_ViewportFrameBuffer);

  // 本次绘制首次绑定FrameBuffer，提交FrameBuffer的清屏命令
  RenderCommand::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, m_ClearColor);

  // 设置视口大小
  RenderCommand::SetViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y);
}

void OpenGLRenderer::EndFrame()
{
  // 解绑FrameBuffer
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
}

void OpenGLRenderer::RenderScene(const std::shared_ptr<Camera> mainCamera,
                                 std::shared_ptr<RenderQueue> renderQueue)
{
  if (!renderQueue) {
    m_Logger->warn("OpenGLRenderer::Render called with null renderQueue");
    return;
  }

  // 1. 获取视图和投影矩阵
  const glm::mat4 viewMatrix = mainCamera->GetViewMatrix();
  const glm::mat4 projectionMatrix = mainCamera->GetProjectionMatrix();

  // 2. 按队列类型顺序渲染（通常：不透明 -> Alpha测试 -> 透明）
  const std::vector<RenderQueue::QueueType> renderOrder = {RenderQueue::QueueType::Opaque,
                                                           RenderQueue::QueueType::AlphaTest,
                                                           RenderQueue::QueueType::Transparent,
                                                           RenderQueue::QueueType::Custom};

  // 3. 遍历所有队列类型
  for (auto queueType : renderOrder) {
    const auto &items = renderQueue->GetItems(queueType);
    if (items.empty())
      continue;

    // 4. 渲染当前队列的所有项
    for (const auto &item : items) {
      if (!item.material || !item.mesh) {
        m_Logger->warn("Invalid renderable item - missing material or mesh");
        continue;
      }

      // 通过RenderCommand提交绘制命令
      RenderCommand::Submit(item, viewMatrix, projectionMatrix);
    }
  }
}

void OpenGLRenderer::SetClearColor(const glm::vec4 &color)
{
  m_ClearColor = color;
}

void OpenGLRenderer::SetViewport(uint32_t width, uint32_t height)
{
  m_ViewportSize = {width, height};

  // 调整FrameBuffer大小
  if (m_ViewportFrameBuffer) {
    m_ViewportFrameBuffer->Resize(width, height);
  }

  // 提交视口设置命令
  RenderCommand::SetViewport(0, 0, width, height);
}

std::shared_ptr<FrameBuffer> OpenGLRenderer::GetViewportFrameBuffer() const
{
  return m_ViewportFrameBuffer;
}

intptr_t OpenGLRenderer::GetViewportFramebufferID() const
{
  // 返回颜色附件0的纹理ID
  return static_cast<intptr_t>(m_ViewportFrameBuffer->GetColorAttachmentID());
}

}  // namespace mite