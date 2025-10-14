#ifndef MITE_OPENGL_DEVICE
#define MITE_OPENGL_DEVICE

#include "render_core/render_device.h"

namespace mite {
/**
 * OpenGL渲染设备实现（IRenderDevice接口）
 * 职责：
 * 1. 创建/销毁OpenGL纹理、缓冲区等GPU资源
 * 2. 管理OpenGL状态和资源生命周期
 * 3. 提供与AssetManager的桥梁
 */
class OpenGLDevice : public RenderDevice {
 public:
  OpenGLDevice();
  ~OpenGLDevice() override;
  void CleanupResources();

  // ---- 纹理操作 ----
  TextureGPUHandle CreateTexture(std::shared_ptr<TextureSourceData> data) override;
  TextureGPUHandle CreateRuntimeTexture(std::shared_ptr<TextureCreateInfo> createInfo) override;
  void DestroyTexture(TextureGPUHandle handle) override;
  void BindRuntimeTexture(RuntimeTextureType type,
                          TextureGPUHandle textureHandle,
                          TextureTarget target = TextureTarget::TEXTURE_2D) const override;
  void BindExternalTexture(ExternalTextureType type,
                           TextureGPUHandle textureHandle,
                           TextureTarget target = TextureTarget::TEXTURE_2D) const override;

  // ---- 模型操作 ----
  ModelGPUHandle CreateModel(std::shared_ptr<ModelSourceData> data) override;
  // TODO:
  // 应当在哪里调用DestroyModel，以实现Model的
  // 生命周期结束后，GPU资源的自动释放？
  //
  // 思路：
  // 构建std::shared_ptr<Model>时，添加删除器，
  // 引用计数归零时自动调用删除器触发DestroyModel
  //
  // 难点：
  // 需要整体梳理std::shared_ptr<Model>的生命周期
  void DestroyModel(ModelGPUHandle model) override;
  void BindMesh(std::shared_ptr<Mesh> mesh) const override;
  void DrawMeshLOD(std::shared_ptr<Mesh> mesh, uint32_t lodLevel) const override;
  void DrawIndexed(uint32_t indexCount,
                   uint32_t indexOffset,
                   GLenum mode = GL_TRIANGLES,
                   GLenum indexType = GL_UNSIGNED_INT) const override;

  // ---- FrameBuffer 操作 ----
  std::shared_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferSpec &spec) override;
  void DestroyFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

  // ---- 全屏四边形（用于延迟光照和后处理） ----
  void CreateFullScreenQuad() override;
  void DrawFullScreenQuad() override;
  void DestroyFullScreenQuad() override;
 private:
  // ---- 事件响应函数 ----
  void OnModelLoaded(ModelLoadEvent &e) override;
  void OnTextureLoaded(TextureLoadEvent &e) override;
  void OnRuntimeTextureCreate(RuntimeTextureCreateEvent &e) override;
  void OnRuntimeTextureDestroyRequest(RuntimeTextureDestroyRequestEvent &e) override;

  // ---- 默认纹理管理 ----
  void InitializeDefaultTextures();
  void CleanupDefaultTextures();
  GLuint CreateDefaultTexture();

  // ---- 辅助方法 ----
  void SetVertexAttributes(const VertexLayout &layout);
  void SetTextureParameters(std::shared_ptr<TextureSourceData> data);
  bool UploadTextureData(std::shared_ptr<TextureSourceData> data, GLuint textureId);
  bool GetGLTextureFormats(TextureFormat textureFormat,
                           GLenum &internalFormat,
                           GLenum &format,
                           GLenum &type);
  void CheckGLError(std::string debugName = "");

  // 资源追踪（用于调试和泄漏检测）
  std::unordered_set<GLuint> m_ActiveTextures;  // 活动纹理集合
  std::unordered_set<GLuint> m_ActiveVAOs;      // 活动VAO集合
  std::unordered_set<GLuint> m_ActiveVBOs;      // 活动VBO集合
  std::unordered_set<GLuint> m_ActiveEBOs;      // 活动EBO集合
  std::unordered_set<GLuint> m_ActiveFBOs;      // 活动FBO集合

  // 全屏四边形VAO
  uint32_t m_ScreenQuadVAO = 0;
  uint32_t m_ScreenQuadVBO = 0;

  // 默认纹理
  GLuint m_WhiteTexture = 0;  // 1x1 白色纹理

  // 日志系统
  Logger m_Logger;
};
};  // namespace mite

#endif
