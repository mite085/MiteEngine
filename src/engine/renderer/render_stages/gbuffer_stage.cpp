#include "gbuffer_stage.h"
#include "basic_instance/material_instance.h"
#include "basic_shader/shader_cache.h"
#include "render_core/render_command.h"
#include "basic_event/render_event.h"
#include "render_opengl/opengl_command.h"

namespace mite {
GBufferStage::GBufferStage() : RenderStage("GBufferStage")
{
  // 配置G-Buffer渲染状态
  SetupGBufferRenderState();

  m_Logger->info("GBufferStage created with G-Buffer rendering configuration");
}

GBufferStage::~GBufferStage()
{
  m_Logger->info("GBufferStage destroyed");
}

void GBufferStage::Initialize()
{
  if (m_Initialized) {
    m_Logger->warn("GBufferStage already initialized");
    return;
  }

  // 创建G-Buffer
  m_GBuffer = std::make_shared<GBuffer>();
  m_GBuffer->create();

  m_Initialized = true;
  m_Logger->info("GBufferStage initialization completed");
}

void GBufferStage::Execute(RenderContext &context)
{
  // 检查初始化和GBuffer状态
  if (!m_Initialized || !m_GBuffer || !m_GBuffer->isValid()) {
    m_Logger->warn("GBufferStage executed but not properly initialized");
    return;
  }

  // 检查上下文
  if (!context.IsValid()) {
    m_Logger->warn("GBufferStage executed with invalid context");
    return;
  }

  // 从上下文获取G-Buffer Stage的着色器
  std::shared_ptr<OpenGLShader> gbufferShader = context.GetStageShader(m_Name);
  if (!gbufferShader) {
    m_Logger->error("G-Buffer Stage: No shader registered for G-Buffer Stage in context");
    return;
  }
  if (gbufferShader->GetProgramId() == 0) {
    m_Logger->error("G-Buffer Stage: G-Buffer Shader from context is not properly linked");
    return;
  }
  
  // 获取上下文记录的帧缓冲尺寸
  glm::uvec2 viewportSize = context.GetViewportSize();

  // 若帧缓冲尺寸不匹配，则使用新的尺寸重新create
  if (m_GBuffer->getFramebuffer()->GetSize() != viewportSize) {
    m_GBuffer->resize(viewportSize.x, viewportSize.y);
  }

  // 绑定G-Buffer为渲染目标
  RenderCommand::Get().BindFrameBuffer(m_GBuffer->getFramebuffer());

  // 清除G-Buffer
  RenderCommand::Get().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                             glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),  // 透明黑色清除
                             1.0f);

  // 绑定着色器
  RenderCommand::Get().BindShader(gbufferShader);

  // stage开始之前初始化并绑定相机UBO
  RenderCommand::Get().BindCameraUBO(context.GetMainCameraInstance());

  // 设置G-Buffer渲染状态
  SetupGBufferRenderState();

  // 渲染各个队列到G-Buffer
  RenderOpaqueQueue(context);
  RenderAlphaTestQueue(context);

  // 解绑G-Buffer
  RenderCommand::Get().UnbindFrameBuffer();

  // 执行渲染操作
  RenderCommand::Get().Flush();

  // 存储渲染结果到上下文（并非渲染命令，这些纹理是提前创建好的，可以提前交给上下文管理）
  for (const auto &type : GBuffer::GetTextureTypes()) {
    context.SetGBufferTexture(m_GBuffer->getTexture(type));
  }

  // 发布绘制完成事件
  for (const auto &type : GBuffer::GetTextureTypes()) {
    RenderCommand::Get().PublishEventRuntimeTextureFinished(m_GBuffer->getTexture(type));
  }

}
void GBufferStage::Shutdown()
{
  if (m_GBuffer) {
    m_GBuffer->cleanup();
  }

  m_Initialized = false;
  m_Logger->info("GBufferStage shutdown completed");
}

void GBufferStage::RenderOpaqueQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  if (!renderQueue) {
    return;
  }

  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::Opaque);
  if (items.empty()) {
    return;
  }

  // 设置不透明物体渲染状态
  RenderCommand::Get().SetRenderState(m_OpaqueState);

  // 渲染所有不透明物体到G-Buffer
  size_t renderedCount = 0;
  size_t skippedCount = 0;

  for (const auto &item : items) {
    if (!ValidateGBufferRenderableItem(item)) {
      skippedCount++;
      continue;
    }

    // 使用G-Buffer着色器提交
    auto gbufferShader = context.GetStageShader(m_Name);
    if (gbufferShader) {
      RenderCommand::Get().SubmitDrawCall(item.mesh, gbufferShader);
      renderedCount++;
    }
    else {
      skippedCount++;
      m_Logger->warn("No G-Buffer shader available for material, skipping renderable item");
    }
  }

  //m_Logger->trace(
  //    "Rendered {} opaque objects to G-Buffer, skipped {}", renderedCount, skippedCount);
}

void GBufferStage::RenderAlphaTestQueue(RenderContext &context)
{
  auto renderQueue = context.GetRenderQueue();
  if (!renderQueue) {
    return;
  }

  const auto &items = renderQueue->GetItems(RenderQueue::QueueType::AlphaTest);
  if (items.empty()) {
    return;
  }

  // 设置Alpha测试物体渲染状态
  RenderCommand::Get().SetRenderState(m_AlphaTestState);

  // 渲染所有Alpha测试物体到G-Buffer
  size_t renderedCount = 0;
  size_t skippedCount = 0;

  for (const auto &item : items) {
    if (!ValidateGBufferRenderableItem(item)) {
      skippedCount++;
      continue;
    }

    // 使用G-Buffer着色器提交
    auto gbufferShader = context.GetStageShader(m_Name);
    if (gbufferShader) {
      RenderCommand::Get().SubmitDrawCall(item.mesh, gbufferShader);
      renderedCount++;
    }
    else {
      skippedCount++;
    }
  }
  m_Logger->trace(
      "Rendered {} alpha-test objects to G-Buffer, skipped {}", renderedCount, skippedCount);
}

void GBufferStage::SetupGBufferRenderState()
{
  // G-Buffer阶段需要深度测试和写入，但不需要混合
  m_OpaqueState = std::make_shared<OpenGLRenderState>();
  m_OpaqueState->depthTest = true;
  m_OpaqueState->depthWrite = true;
  m_OpaqueState->blend = false;
  m_OpaqueState->cullFace = true;
  m_OpaqueState->colorWriteR = true;
  m_OpaqueState->colorWriteG = true;
  m_OpaqueState->colorWriteB = true;
  m_OpaqueState->colorWriteA = true;

  // Alpha测试使用相同状态
  m_AlphaTestState = std::make_shared<OpenGLRenderState>();
  m_AlphaTestState->depthTest = true;
  m_AlphaTestState->depthWrite = true;
  m_AlphaTestState->blend = false;
  m_AlphaTestState->cullFace = true;
  m_AlphaTestState->colorWriteR = true;
  m_AlphaTestState->colorWriteG = true;
  m_AlphaTestState->colorWriteB = true;
  m_AlphaTestState->colorWriteA = true;
}

bool GBufferStage::ValidateGBufferRenderableItem(const RenderableItem &item) const
{
  // 验证基础有效性
  if (!item.material) {
    m_Logger->trace("GBufferStage: Renderable item has no material");
    return false;
  }

  // 验证网格有效性
  if (item.mesh->GetMesh()->GetModelHandle().vertexArray == 0) {
    m_Logger->trace("GBufferStage: Renderable item has invalid mesh");
    return false;
  }

  // 添加G-Buffer特定的验证逻辑
  // 例如：检查材质是否支持G-Buffer渲染（目前创建的材质均支持，所以忽略这一步）

  return true;
}

}  // namespace mite