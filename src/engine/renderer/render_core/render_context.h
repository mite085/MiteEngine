#ifndef MITE_RENDER_CONTEXT
#define MITE_RENDER_CONTEXT

#include "basic_data/camera.h"
#include "basic_data/framebuffer.h"
#include "render_queue.h"

namespace mite {

/**
 * @brief 渲染上下文（统一数据传递和资源共享）
 *
 * 职责：
 * 1. 封装渲染所需的所有数据（场景、相机、FrameBuffer等）
 * 2. 提供阶段间的数据共享机制
 * 3. 管理临时渲染资源
 * 4. 提供设备访问接口
 */
class RenderContext {
 public:
  explicit RenderContext();
  ~RenderContext();

  // ---- 场景数据设置 ----
  void SetSceneData(std::shared_ptr<RenderQueue> renderQueue,
                    const glm::mat4 &viewMatrix,
                    const glm::mat4 &projectionMatrix);

  void SetCameraData(const glm::mat4 &viewMatrix,
                     const glm::mat4 &projectionMatrix,
                     const glm::vec3 &cameraPosition = glm::vec3(0.0f));

  // ---- 渲染目标设置 ----
  void SetFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer);
  void SetViewport(const glm::ivec2 &size);

  // ---- 数据访问接口 ----

  // 场景数据
  std::shared_ptr<RenderQueue> GetRenderQueue() const
  {
    return m_RenderQueue;
  }
  const glm::mat4 &GetViewMatrix() const
  {
    return m_ViewMatrix;
  }
  const glm::mat4 &GetProjectionMatrix() const
  {
    return m_ProjectionMatrix;
  }
  const glm::vec3 &GetCameraPosition() const
  {
    return m_CameraPosition;
  }

  // 渲染目标
  std::shared_ptr<FrameBuffer> GetFrameBuffer() const
  {
    return m_CurrentFrameBuffer;
  }
  glm::ivec2 GetViewportSize() const
  {
    return m_ViewportSize;
  }

  // ---- 临时资源管理 ----
  template<typename T>
  void SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource);

  template<typename T> std::shared_ptr<T> GetTemporaryResource(const std::string &name) const;

  void ClearTemporaryResources();

  // ---- 上下文状态 ----
  bool IsValid() const;
  void Validate() const;

 private:
  // ---- 场景数据 ----
  std::shared_ptr<RenderQueue> m_RenderQueue;
  glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
  glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  glm::vec3 m_CameraPosition = glm::vec3(0.0f);

  // ---- 渲染目标 ----
  std::shared_ptr<FrameBuffer> m_CurrentFrameBuffer;
  glm::ivec2 m_ViewportSize = {1280, 720};

  // ---- 临时资源存储 ----
  std::unordered_map<std::string, std::shared_ptr<void>> m_TemporaryResources;

  Logger m_Logger;
};

// 模板实现
template<typename T>
void RenderContext::SetTemporaryResource(const std::string &name, std::shared_ptr<T> resource)
{
  if (name.empty()) {
    m_Logger->warn("Attempted to set temporary resource with empty name");
    return;
  }

  // 转换为void指针存储，但保持类型安全通过模板管理
  std::shared_ptr<void> voidResource = std::static_pointer_cast<void>(resource);
  m_TemporaryResources[name] = voidResource;

  m_Logger->debug("Set temporary resource: {}", name);
}

template<typename T>
std::shared_ptr<T> RenderContext::GetTemporaryResource(const std::string &name) const
{
  auto it = m_TemporaryResources.find(name);
  if (it == m_TemporaryResources.end()) {
    m_Logger->debug("Temporary resource not found: {}", name);
    return nullptr;
  }

  try {
    // 尝试转换回原始类型
    std::shared_ptr<T> typedResource = std::static_pointer_cast<T>(it->second);
    return typedResource;
  }
  catch (const std::bad_cast &e) {
    m_Logger->error("Failed to cast temporary resource '{}': {}", name, e.what());
    return nullptr;
  }
}

}  // namespace mite

#endif
