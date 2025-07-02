#ifndef MITE_OPENGL_DEVICE
#define MITE_OPENGL_DEVICE

#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后
#include "render_device.h"

namespace mite {
/**
 * OpenGL渲染设备实现（IRenderDevice接口）
 * 职责：
 * 1. 创建/销毁OpenGL纹理、缓冲区等GPU资源
 * 2. 管理OpenGL状态和资源生命周期
 * 3. 提供与AssetManager的桥梁
 */
class OpenGLDevice : public IRenderDevice {
 public:
  OpenGLDevice();
  ~OpenGLDevice() override;

  // ---- 纹理操作 ----
  TextureGPUHandle CreateTexture(const TextureAsset &texture) override;
  void DestroyTexture(TextureGPUHandle handle) override;
  void BindTexture(TextureGPUHandle handle, uint32_t slot) const override;
  void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) override;
  void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) override;

  // ---- 模型操作 ----
  ModelGPUHandle CreateModel(const ModelAsset &model) override;
  void DestroyModel(ModelGPUHandle model) override;
  // 注意：
  // 由于Asset仅维护Model，由Model维护Mesh，
  // 所以Create和Destroy接收的是Model数据。
  // 但Bind和Draw的操作是和Mesh强相关，
  // 所以这里实现Bind Mesh而非Bind Model
  void BindMesh(MeshGPUHandle handle) const override;
  void DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const override;

 private:
  // ---- 辅助方法 ----
  GLenum ConvertWrapMode(TextureWrapMode mode) const;
  void ConvertFilterMode(TextureFilterMode mode, GLenum &outMinFilter, GLenum &outMagFilter) const;
  static GLenum TranslateTextureFormat(TextureFormat format);

  // 创建单个SubMesh的GPU资源
  MeshGPUHandle CreateSubMesh(const MeshData &subMesh);

  // 资源追踪（用于调试和泄漏检测）
  std::unordered_set<GLuint> activeTextures_;
  std::unordered_set<GLuint> activeMeshsVAO_, activeMeshsVBO_, activeMeshsEBO_;
};
};  // namespace mite

#endif
