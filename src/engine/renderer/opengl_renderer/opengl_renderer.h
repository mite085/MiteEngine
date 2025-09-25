#ifndef MITE_OPENGL_RENDERER
#define MITE_OPENGL_RENDERER

#include "opengl_renderer/opegl_device.h"
#include "renderer.h"

#include "basic_data/model.h"
#include "basic_data/texture.h"
#include "renderable_item.h"

namespace mite {
/**
 * OpenGL渲染器实现类
 * 职责：
 * 1. 实现基类定义的渲染接口
 * 2. 管理OpenGL专属状态（如VAO、Shader Program）
 * 3. 集成FrameBuffer系统
 */
class OpenGLRenderer : public Renderer {
 public:
  explicit OpenGLRenderer();
  ~OpenGLRenderer() override;

  // ---- 初始化 ----
  void Initialize() override;

  // ---- 帧控制 ----
  void BeginFrame() override;
  void EndFrame() override;

  // ---- 场景渲染 ----
  void RenderScene(std::shared_ptr<RenderQueue> renderQueue,
                   const glm::mat4 viewMatrix,
                   const glm::mat4 projectionMatrix) override;

  // ---- 状态设置 ----
  void SetClearColor(const glm::vec4 &color) override;

  // ---- UI接口 ----
  std::shared_ptr<FrameBuffer> GetMainFrameBuffer() const override;
  std::shared_ptr<FrameBuffer> GetDisplayFrameBuffer() const override;

 private:
  // ---- 私有方法 ----
  /**
   * @brief 创建默认FrameBuffer
   */
  void CreateDefaultFrameBuffer();
  /**
   * @brief 双缓冲管理
   */
  void SwapFrameBuffers();

  // ---- 双缓冲成员变量 ----
  std::shared_ptr<FrameBuffer> m_MainFrameBuffer;     // 主渲染缓冲（用于3D场景渲染）
  std::shared_ptr<FrameBuffer> m_DisplayFrameBuffer;  // 显示缓冲（用于UI显示）
  bool m_IsRenderingScene = false;                    // 标记当前渲染阶段
  Logger m_Logger;                                     // 日志系统
};
}  // namespace mite

#endif
