#include "render_context.h"

namespace mite {

RenderContext::RenderContext() : m_Logger(LoggerSystem::CreateModuleLogger("Mite Render Context"))
{
  m_Logger->info("Render Context created");
}

RenderContext::~RenderContext()
{
  ClearTemporaryResources();
  m_Logger->info("Render Context destroyed");
}

void RenderContext::SetSceneData(std::shared_ptr<RenderQueue> renderQueue,
                                 const glm::mat4 &viewMatrix,
                                 const glm::mat4 &projectionMatrix)
{
  m_RenderQueue = renderQueue;
  m_ViewMatrix = viewMatrix;
  m_ProjectionMatrix = projectionMatrix;

  //m_Logger->debug("Set scene data - RenderQueue: {}, Camera position: ({}, {}, {})",
  //                m_RenderQueue ? "valid" : "null",
  //                m_CameraPosition.x,
  //                m_CameraPosition.y,
  //                m_CameraPosition.z);
}

void RenderContext::SetFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
  m_CurrentFrameBuffer = framebuffer;

  //m_Logger->debug("Set framebuffer - Size: {}x{}", m_ViewportSize.x, m_ViewportSize.y);
}

void RenderContext::ClearTemporaryResources()
{
  size_t count = m_TemporaryResources.size();
  m_TemporaryResources.clear();

  if (count > 0) {
    m_Logger->debug("Cleared {} temporary resources", count);
  }
}

bool RenderContext::IsValid() const
{
  return m_RenderQueue != nullptr && m_CurrentFrameBuffer != nullptr;
}

void RenderContext::Validate() const
{
  if (!m_RenderQueue) {
    throw std::runtime_error("RenderContext validation failed: No render queue");
  }
  if (!m_CurrentFrameBuffer) {
    throw std::runtime_error("RenderContext validation failed: No framebuffer");
  }

  m_Logger->debug("RenderContext validation passed");
}

}  // namespace mite
