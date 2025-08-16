#ifndef MITE_RENDERER_API
#define MITE_RENDERER_API

#include "basic_data/camera.h"
#include "basic_data/model.h"
#include "basic_data/framebuffer.h"
#include "renderable_item.h"
#include "render_command.h"

namespace mite {
/**
 * 渲染器抽象基类（多后端兼容）
 * 职责：
 * 1. 管理渲染管线状态（Shader、FBO等）
 * 2. 协调AssetManager与IRenderDevice的交互
 * 3. 提供高层渲染接口（不直接接触OpenGL/Vulkan API）
 *
 * 修改点：
 * 1. 增加FrameBuffer支持
 * 2. 为ViewportPanel提供接口
 */
class Renderer {
 public:
  explicit Renderer();
  virtual ~Renderer() = default;

  // ---- 初始化 ----
  virtual void Initialize() = 0;

  // ---- 帧控制 ----
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  /**
   * @brief 渲染场景的核心接口
   * @param mainCamera 主摄像机
   * @param renderQueue 从SceneView获取的可渲染实体列表
   * 
   * RenderScene()仅负责提交RenderCommand队列，
   * EndFrame()负责调用RenderCommand::Flush();执行所有命令
   */
  virtual void RenderScene(const std::shared_ptr<Camera> mainCamera,
                           const std::vector<std::shared_ptr<RenderableItem>> &renderQueue) = 0;

  // ---- 状态设置 ----
  virtual void SetClearColor(const glm::vec4 &color) = 0;
  virtual void SetViewport(uint32_t width, uint32_t height) = 0;

  // ---- 供UI模块调用的接口 ----
  /**
   * @brief 获取视口FrameBuffer
   * @return FrameBuffer智能指针
   */
  virtual std::shared_ptr<FrameBuffer> GetViewportFrameBuffer() const = 0;

  /**
   * @brief 获取视口FrameBuffer的纹理ID（兼容旧接口）
   * @return 纹理ID的intptr_t表示
   */
  virtual intptr_t GetViewportFramebufferID() const = 0;

 protected:
  glm::vec4 clearColor_ = {0.1f, 0.1f, 0.1f, 1.0f};  // 清屏颜色
  glm::ivec2 viewportSize_ = {1280, 720};            // 视口尺寸
};
}  // namespace mite

#endif