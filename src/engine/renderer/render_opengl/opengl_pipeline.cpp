#include "opengl_pipeline.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_cache.h"
#include "filesystem/filesystem.h"
#include "opengl_command.h"
#include "render_stages/blend_stage.h"
#include "render_stages/deferred_lighting_stage.h"
#include "render_stages/forward_stage.h"
#include "render_stages/gbuffer_stage.h"
#include "render_stages/shadow_map_stage.h"

namespace mite {
OpenGLPipeline::OpenGLPipeline() : RenderPipeline()
{
  m_Logger->info("OpenGL Pipeline created");

  // 填充默认状态
  SetupDefaultRenderState();

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

  // 使用默认纹理绑定所有分配的纹理槽位，避免使用阶段可能出现的纹理槽位悬空问题
  for (auto bindPoint : BindingPointManager::Get().GetAllocatedTextureBindings()) {
    RenderCommand::Get().BindDefaultTexture(bindPoint);
  }

  // 创建渲染上下文
  m_Context = std::make_unique<RenderContext>();

  // 添加ShadowMap Stage
  std::shared_ptr<OpenGLShader> shadowMapShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/shadowmap/shadowmap.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/shadowmap/shadowmap.frag.glsl").string());
  AddStage(std::make_unique<ShadowMapStage>(), shadowMapShader);

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
  AddStage(std::make_unique<ForwardStage>(), forwardShader);

  // 添加Blend Render Stage
  std::shared_ptr<OpenGLShader> blendShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/blend/blend.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/blend/blend.frag.glsl").string());
  AddStage(std::make_unique<BlendStage>(), blendShader);

  // 初始化所有阶段
  for (auto &stage : m_Stages) {
    if (stage->IsEnabled()) {
      stage->Initialize(*m_Context);
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

  // 重置状态为默认值（避免上一帧最后stage的状态对当前帧造成状态污染）
  RenderCommand::Get().SetRenderState(m_DefaultState);

  // 设置上下文视口尺寸
  if (m_ShouldResize) {
    m_Context->SetViewportSize(static_cast<uint32_t>(glm::max(m_PendingSize.x, 0.0f)),
                               static_cast<uint32_t>(glm::max(m_PendingSize.y, 0.0f)));
    m_ShouldResize = false;
  }

  // 使用PendingSize作为当前视口尺寸（每帧仅需一次设定）
  RenderCommand::Get().SetViewport(0,
                                   0,
                                   static_cast<uint32_t>(glm::max(m_PendingSize.x, 0.0f)),
                                   static_cast<uint32_t>(glm::max(m_PendingSize.y, 0.0f)));

  m_Logger->trace("Pipeline BeginFrame completed");
}

void OpenGLPipeline::EndFrame()
{
  // 结束场景渲染阶段
  m_IsRenderingScene = false;

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

   m_Logger->trace("Pipeline EndFrame completed");
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

   m_Logger->trace("Pipeline RenderScene completed");
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

void OpenGLPipeline::SetupDefaultRenderState()
{
  // 默认OpenGL全局状态
  m_DefaultState = std::make_shared<OpenGLRenderState>();
  m_DefaultState->depthTest = true;
  m_DefaultState->depthWrite = true;
  m_DefaultState->blend = true;
  m_DefaultState->cullFace = true;
  m_DefaultState->colorWriteR = true;
  m_DefaultState->colorWriteG = true;
  m_DefaultState->colorWriteB = true;
  m_DefaultState->colorWriteA = true;
}
}  // namespace mite