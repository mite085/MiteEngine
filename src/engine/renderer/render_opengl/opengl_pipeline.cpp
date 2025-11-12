#include "opengl_pipeline.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_cache.h"
#include "render_stages/forward_stage.h"
#include "render_stages/deferred_lighting_stage.h"
#include "render_stages/gbuffer_stage.h"
#include "render_stages/shadow_map_stage.h"
#include "filesystem/filesystem.h"

namespace mite {
OpenGLPipeline::OpenGLPipeline() : RenderPipeline()
{
  m_Logger->info("OpenGL Pipeline created");

  // Viewport Resize事件订阅
  m_EventSubscriptions.SubscribeImmediate<ViewportResizeEvent>(BIND_DISPATCH_FN(OnViewPortResize));
}

OpenGLPipeline::~OpenGLPipeline()
{
  m_Logger->info("OpenGL Pipeline destroyed");
}

void OpenGLPipeline::Initialize()
{
  // 初始化RenderCommand
  RenderCommand::Get().Init();

  // 创建渲染上下文
  m_Context = std::make_unique<RenderContext>();

  // 添加ShadowMap Stage
  std::shared_ptr<OpenGLShader> shadowMapShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/shadowmap/shadowmap.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/shadowmap/shadowmap.frag.glsl").string());
  //AddStage(std::make_unique<ShadowMapStage>(), shadowMapShader);

  // 添加G-Buffer Stage
  std::shared_ptr<OpenGLShader> gBufferShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/gbuffer/gbuffer.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/gbuffer/gbuffer.frag.glsl").string());
  AddStage(std::make_unique<GBufferStage>(), gBufferShader);

  // 添加Deferred Lighting Stage
  std::shared_ptr<OpenGLShader> deferredLightingShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/lighting/deferred_lighting.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/lighting/deferred_lighting.frag.glsl").string());
  AddStage(std::make_unique<DeferredLightingStage>(), deferredLightingShader);

  // 添加Forward Render Stage
  std::shared_ptr<OpenGLShader> forwardShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/forward/forward.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/forward/forward.frag.glsl").string());
  //AddStage(std::make_unique<ForwardStage>(), forwardShader);

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

  // 清屏命令
  RenderCommand::Get().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, m_ClearColor);

  // 设置上下文视口尺寸
  if (m_ShouldResize) {
    m_Context->SetViewportSize(static_cast<uint32_t>(glm::max(m_PendingSize.x, 0.0f)),
                               static_cast<uint32_t>(glm::max(m_PendingSize.y, 0.0f)));
    m_ShouldResize = false;
  }

  // 使用PendingSize作为当前视口尺寸（每帧仅需一次设定）
  RenderCommand::Get().SetViewport(0, 0, static_cast<uint32_t>(glm::max(m_PendingSize.x, 0.0f)), static_cast<uint32_t>(glm::max(m_PendingSize.y, 0.0f)));

  // m_Logger->debug("Pipeline BeginFrame completed");
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

  // m_Logger->debug("Pipeline EndFrame completed");
}

void OpenGLPipeline::RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                                 std::shared_ptr<CameraInstance> cameraInstance)
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
  m_Context->SetSceneData(renderQueue, cameraInstance);

  // 执行所有启用的阶段
  for (auto &stage : m_Stages) {
    if (stage->IsEnabled()) {
      stage->Execute(*m_Context);
    }
  }

  // m_Logger->debug("Pipeline RenderScene completed");
}

void OpenGLPipeline::SetClearColor(const glm::vec4 &color)
{
  m_ClearColor = color;
}

void OpenGLPipeline::OnViewPortResize(ViewportResizeEvent &event)
{
  if (m_PendingSize != event.GetSize()) {
    m_PendingSize = event.GetSize();
    m_ShouldResize = true;

    event.SetResult(EventResult::Handled);
    return;
  }

  // 尺寸匹配，Resize无效
  event.SetResult(EventResult::Failed);
  return;
}

// std::shared_ptr<FrameBuffer> OpenGLPipeline::GetMainFrameBuffer() const
//{
//   return m_MainFrameBuffer;
// }
//
// std::shared_ptr<FrameBuffer> OpenGLPipeline::GetDisplayFrameBuffer() const
//{
//   return m_DisplayFrameBuffer;
// }

// void OpenGLPipeline::CreateDefaultFrameBuffer()
//{
//   // 创建FrameBuffer规格
//   FrameBufferSpec spec;
//   spec.attachments = {
//       {FrameBufferAttachmentType::Color, TextureFormat::RGBA8},               // 颜色附件
//       {FrameBufferAttachmentType::Depth, TextureFormat::DEPTH_COMPONENT24}};  // 深度附件
//
//   // 创建两个相同的FrameBuffer用于双缓冲
//   m_MainFrameBuffer = std::make_shared<FrameBuffer>(spec);
//   m_DisplayFrameBuffer = std::make_shared<FrameBuffer>(spec);
//
//   if (!m_MainFrameBuffer->IsComplete() || !m_DisplayFrameBuffer->IsComplete()) {
//     m_Logger->error("Failed to create complete framebuffers for double buffering");
//     throw std::runtime_error("Framebuffers are incomplete");
//   }
//
//   m_Logger->info("Created default framebuffer");
// }
//
// void OpenGLPipeline::SwapFrameBuffers()
//{
//   // 直接交换指针
//   std::swap(m_MainFrameBuffer, m_DisplayFrameBuffer);
//
//   // 处理尺寸同步事件
//   if (m_ShouldResize) {
//     // 此时已经进入下一帧，displayBuffer为上一帧准备好的mainBuffer，
//     // 之前准备过程中已经将当前display的size调整过了，仅需调整下一帧的main即可
//     m_MainFrameBuffer->Resize(m_DisplayFrameBuffer->GetSize().x,
//                               m_DisplayFrameBuffer->GetSize().y);
//     m_ShouldResize = false;
//   }
//
//   // m_Logger->debug("Pipeline swapped buffers");
// }
}  // namespace mite