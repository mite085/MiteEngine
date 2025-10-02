#include "gbuffer_stage.h"
#include "basic_instance/material_instance.h"
#include "basic_shader/shader_cache.h"
#include "render_core/render_command.h"

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

  // 加载G-Buffer着色器（预留接口）
  m_GBufferShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/gbuffer/gbuffer.vert.glsl").string(),
      FileSystem::GetAssetPath("shaders/gbuffer/gbuffer.vert.glsl").string());

  if (!m_GBufferShader) {
    m_Logger->warn("No G-Buffer shader set, will need to be provided externally");
  }

  m_Initialized = true;
  m_Logger->info("GBufferStage initialization completed");
}

void GBufferStage::Execute(RenderContext &context)
{
  // 检查初始化和GBuffer状态
  if (!m_Initialized || !m_GBuffer) {
    m_Logger->warn("GBufferStage executed but not properly initialized");
    return;
  }

  // 检查上下文
  if (!context.IsValid()) {
    m_Logger->warn("GBufferStage executed with invalid context");
    return;
  }

  // 获取上下文记录的帧缓冲尺寸
  glm::uvec2 viewportSize = context.GetFrameBuffer()->GetSize();
  // 若GBuffer不可用 / 与帧缓冲尺寸不匹配，则使用新的尺寸重新create
  if (!m_GBuffer->isValid() || m_GBuffer->getFramebuffer()->GetSize() != viewportSize) {
    m_GBuffer->create(viewportSize.x, viewportSize.y);
  }

  // 绑定G-Buffer为渲染目标
  RenderCommand::Get().BindFrameBuffer(m_GBuffer->getFramebuffer());

  // 设置视口
  RenderCommand::Get().SetViewport(0, 0, viewportSize.x, viewportSize.y);

  // 清除G-Buffer
  RenderCommand::Get().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                             glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),  // 透明黑色清除
                             1.0f);

  // 设置G-Buffer渲染状态
  SetupGBufferRenderState();

  // 渲染各个队列到G-Buffer
  RenderOpaqueQueue(context);
  RenderAlphaTestQueue(context);

  // 解绑G-Buffer
  RenderCommand::Get().UnbindFrameBuffer();

  // 将G-Buffer存储到上下文供后续阶段使用
  context.SetTemporaryResource("GBuffer", m_GBuffer);
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

    // 使用G-Buffer专用着色器提交
    auto gbufferShader = GetGBufferShaderForMaterial(item);
    if (gbufferShader) {
      RenderCommand::Get().SubmitToGBuffer(
          item, context.GetViewMatrix(), context.GetProjectionMatrix(), gbufferShader);
      renderedCount++;
    }
    else {
      skippedCount++;
      m_Logger->warn("No G-Buffer shader available for material, skipping renderable item");
    }
  }

  m_Logger->trace(
      "Rendered {} opaque objects to G-Buffer, skipped {}", renderedCount, skippedCount);
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

    // 使用G-Buffer专用着色器提交
    auto gbufferShader = GetGBufferShaderForMaterial(item);
    if (gbufferShader) {
      RenderCommand::Get().SubmitToGBuffer(
          item, context.GetViewMatrix(), context.GetProjectionMatrix(), gbufferShader);
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
  m_OpaqueState.depthTest = true;
  m_OpaqueState.depthWrite = true;
  m_OpaqueState.blend = false;
  m_OpaqueState.cullFace = true;
  m_OpaqueState.colorWriteR = true;
  m_OpaqueState.colorWriteG = true;
  m_OpaqueState.colorWriteB = true;
  m_OpaqueState.colorWriteA = true;

  // Alpha测试使用相同状态
  m_AlphaTestState = m_OpaqueState;
}

bool GBufferStage::ValidateGBufferRenderableItem(const RenderableItem &item) const
{
  // 验证基础有效性
  if (!item.material) {
    m_Logger->trace("GBufferStage: Renderable item has no material");
    return false;
  }

  auto shader = item.material->GetShader();
  if (!shader || shader->GetHandle().programId == 0) {
    m_Logger->trace("GBufferStage: Renderable item has invalid shader");
    return false;
  }

  if (item.mesh.GetModelHandle().vertexArray == 0) {
    m_Logger->trace("GBufferStage: Renderable item has invalid mesh");
    return false;
  }

  // TODO: 添加G-Buffer特定的验证逻辑
  // 例如：检查材质是否支持G-Buffer渲染

  return true;
}

void GBufferStage::EncodeMaterialToGBuffer(const RenderableItem &item)
{
  // TODO: 实现材质参数到G-Buffer的编码逻辑
  // 这将涉及将PBR/NPR参数映射到G-Buffer的特定通道
  // 目前作为预留接口
}

std::shared_ptr<OpenGLShader> GBufferStage::GetGBufferShaderForMaterial(const RenderableItem &item)
{
  // TODO: 实现基于材质的G-Buffer着色器选择逻辑
  // 目前返回默认的G-Buffer着色器
  // 未来可以根据materialType选择不同的G-Buffer着色器变体

  return m_GBufferShader;
}
}  // namespace mite