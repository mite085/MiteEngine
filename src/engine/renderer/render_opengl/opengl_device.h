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
  void DestroyTexture(TextureGPUHandle handle) override;
  void BindTexture(TextureGPUHandle handle, uint32_t slot) const override;

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
  void BindMesh(Mesh mesh) const override;
  uint32_t SelectMeshLODLevel(Mesh mesh,
                              const glm::vec3 &cameraPosition,
                              const glm::mat4 &worldTransform,
                              const glm::mat4 &viewProjectionMatrix,
                              float screenWidth,
                              float lodBias) const override;
  void DrawMeshLOD(Mesh mesh, uint32_t lodLevel) const override;
  void DrawIndexed(uint32_t indexCount,
                   uint32_t indexOffset,
                   GLenum mode = GL_TRIANGLES,
                   GLenum indexType = GL_UNSIGNED_INT) const override;

  // ---- FrameBuffer 操作 ----
  FrameBuffer::Ptr CreateFrameBuffer(const FrameBufferSpec &spec) override;
  void DestroyFrameBuffer(FrameBuffer::Ptr framebuffer) override;

 private:
  // ---- 事件响应函数 ----
  void OnModelLoaded(ModelLoadEvent &e) override;
  void OnTextureLoaded(TextureLoadEvent &e) override;

  // ---- 辅助方法 ----
  void SetVertexAttributes(const VertexLayout &layout);
  void SetTextureParameters(std::shared_ptr<TextureSourceData> data);
  bool UploadTextureData(std::shared_ptr<TextureSourceData> data, GLuint textureId);
  bool GetGLTextureFormats(TextureFormat textureFormat,
                           GLenum &internalFormat,
                           GLenum &format,
                           GLenum &type);
  void CheckGLError();

  // 资源追踪（用于调试和泄漏检测）
  std::unordered_set<GLuint> m_ActiveTextures;  // 活动纹理集合
  std::unordered_set<GLuint> m_ActiveVAOs;      // 活动VAO集合
  std::unordered_set<GLuint> m_ActiveVBOs;      // 活动VBO集合
  std::unordered_set<GLuint> m_ActiveEBOs;      // 活动EBO集合
  std::unordered_set<GLuint> m_ActiveFBOs;      // 活动FBO集合

  // 日志系统
  Logger m_Logger;
};
};  // namespace mite

#endif
