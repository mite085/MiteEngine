#ifndef MITE_OPENGL_COMMAND
#define MITE_OPENGL_COMMAND

#include "render_core/render_command.h"

namespace mite {
// OpenGL特定的渲染状态扩展（大部分情况下使用默认值即可）
class OpenGLRenderState : public RenderState {
 public:
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

  // ---------------- 基础命令接口 ----------------
  void Init() override;
  void Clear(uint32_t clearFlags,
             const glm::vec4 &clearColor,
             float depthClear,
             int stencilClear) override;
  void BindFrameBuffer(const std::shared_ptr<FrameBuffer> &framebuffer) override;
  void UnbindFrameBuffer() override;
  void SetViewport(int x, int y, int width, int height) override;
  void SetRenderState(const std::shared_ptr<RenderState> &state) override;

  // ---------------- 原子操作命令 ----------------
  void BindCameraUBO(CameraInstance &instance) override;
  void BindMaterialUBO(MaterialInstance &instance) override;
  void BindLightSSBO(LightShaderStorgeBuffer &instance) override;
  void BindShader(
      std::shared_ptr<OpenGLShader> shader,
      std::function<void(std::shared_ptr<OpenGLShader>)> uniformSetup = nullptr) override;
  void UnbindShader(std::shared_ptr<OpenGLShader> shader) override;
  void BindRuntimeTexture(RuntimeTextureType type,
                          TextureGPUHandle textureHandle,
                          TextureTarget target = TextureTarget::TEXTURE_2D) override;
  void BindExternalTexture(ExternalTextureType type,
                           TextureGPUHandle textureHandle,
                           TextureTarget target = TextureTarget::TEXTURE_2D) override;
  void BindMesh(std::shared_ptr<Mesh> mesh) override;
  void DrawMesh(uint32_t indexCount,
                uint32_t indexOffset = 0,
                uint32_t primitiveType = 0x0004,
                uint32_t indexType = 0x1405) override;

  // ---------------- 整合操作命令 ----------------
  void SubmitDrawCall(std::shared_ptr<MeshInstance> meshInstance,
                      std::shared_ptr<OpenGLShader> shader) override;

  // ---------------- 完成事件发布 ----------------
  void PublishEventRuntimeTextureFinished(RuntimeTexturePtr texture,
                                          std::string identify) override;

  // ---------------- 执行控制 ----------------
  void Flush() override;
  void ClearQueue() override;

  // OpenGL特定方法
  void SetRenderState(const OpenGLRenderState &state);

 private:
  OpenGLRenderState m_CurrentGLState;
  Logger m_Logger;

  // 辅助方法
  void ApplyOpenGLState(const OpenGLRenderState &state);
  static void CheckGLError(std::string debugName = "");
};
}  // namespace mite

#endif
