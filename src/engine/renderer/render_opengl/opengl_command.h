#ifndef MITE_OPENGL_COMMAND
#define MITE_OPENGL_COMMAND

#include "render_core/render_command.h"

namespace mite {

// OpenGL特定的渲染状态扩展（大部分情况下使用默认值即可）
struct OpenGLRenderState : public RenderState {
  GLenum depthFunc = GL_LESS;
  GLenum blendSrc = GL_SRC_ALPHA;
  GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;
  GLenum cullFaceMode = GL_BACK;
  GLenum alphaFunc = GL_GREATER;  // Alpha测试函数

  // 模板测试参数
  GLenum stencilFunc = GL_ALWAYS;
  GLint stencilRef = 0;
  GLuint stencilMask = 0xFF;
  GLenum stencilFail = GL_KEEP;
  GLenum stencilPassDepthFail = GL_KEEP;
  GLenum stencilPassDepthPass = GL_KEEP;
};

/**
 * @brief OpenGL渲染命令具体实现
 *
 * 修改说明：
 * 1. 继承自CoreRenderCommand，实现平台相关细节
 * 2. 保留原有RenderCommand的所有功能
 * 3. 添加GLenum等平台特定类型
 */
class OpenGLRenderCommand : public RenderCommand {
 public:
  OpenGLRenderCommand() = default;

  // ---- 实现核心接口 ----
  void Init() override;
  void Clear(uint32_t clearFlags,
             const glm::vec4 &clearColor,
             float depthClear,
             int stencilClear) override;
  void BindFrameBuffer(const FrameBuffer::Ptr &framebuffer) override;
  void UnbindFrameBuffer() override;
  void Submit(RenderableItem item, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) override;
  void SetViewport(int x, int y, int width, int height) override;
  void SetRenderState(const RenderState &state) override;
  void Flush() override;
  void ClearQueue() override;

  // OpenGL特定方法
  void SetRenderState(const OpenGLRenderState &state);

 private:
  // 修改：使用OpenGL特定的状态结构
  OpenGLRenderState m_CurrentGLState;

  Logger m_Logger;

  // 辅助方法
  void ApplyOpenGLState(const OpenGLRenderState &state);

  static void CheckGLError();
};

}  // namespace mite

#endif
