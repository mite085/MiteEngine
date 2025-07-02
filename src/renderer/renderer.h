#ifndef MITE_RENDERER_API
#define MITE_RENDERER_API

#include "headers/headers.h"
#include "glm/glm.hpp"
#include "render_device.h"
#include "asset_manager.h"

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
  explicit Renderer(std::shared_ptr<AssetManager> assetManager);
  virtual ~Renderer() = default;
  virtual void Initialize() = 0;

  // ---- 资源管理 ----
  virtual void LoadTexture(AssetID id) = 0;
  virtual void UnloadTexture(AssetID id) = 0;
  virtual void LoadModel(AssetID id) = 0;
  virtual void UnloadModel(AssetID id) = 0;

  // ---- 渲染指令 ----
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;
  virtual void DrawModel(AssetID modelId, const glm::mat4 &transform) = 0;

  // ---- 状态设置 ----
  void SetClearColor(const glm::vec4 &color);
  void SetViewport(uint32_t width, uint32_t height);

 protected:
  std::shared_ptr<AssetManager> assetManager_;
  glm::vec4 clearColor_ = {0.1f, 0.1f, 0.1f, 1.0f};
  glm::ivec2 viewportSize_ = {1280, 720};
};

}  // namespace mite

#endif