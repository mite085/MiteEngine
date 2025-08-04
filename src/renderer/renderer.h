#ifndef MITE_RENDERER_API
#define MITE_RENDERER_API

#include "data/model.h"
#include "renderable_entity.h"

namespace mite {
/**
 * 渲染器抽象基类（多后端兼容）
 * 职责：
 * 1. 管理渲染管线状态（Shader、FBO等）
 * 2. 协调AssetManager与IRenderDevice的交互
 * 3. 提供高层渲染接口（不直接接触OpenGL/Vulkan API）
 */
class Renderer {
 public:
  explicit Renderer();
  virtual ~Renderer() = default;
  virtual void Initialize() = 0;

  // ---- 渲染指令 ----
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  /**
   * 渲染场景的核心接口
   * @param renderQueue 从SceneView获取的可渲染实体列表
   */
  virtual void RenderScene(const std::vector<RenderableEntity> &renderQueue) = 0;

  // ---- 状态设置 ----
  virtual void SetClearColor(const glm::vec4 &color) = 0;
  virtual void SetViewport(uint32_t width, uint32_t height) = 0;

  // ---- 供Window调用的接口 ----
  virtual intptr_t GetViewportFramebuffer() = 0;

 protected:
  glm::vec4 clearColor_ = {0.1f, 0.1f, 0.1f, 1.0f};
  glm::ivec2 viewportSize_ = {1280, 720};
};
}  // namespace mite

#endif