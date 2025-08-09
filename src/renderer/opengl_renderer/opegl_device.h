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
  TextureGPUHandle CreateTexture(const TextureSourceData &data) override;
  void DestroyTexture(TextureGPUHandle handle) override;
  void BindTexture(TextureGPUHandle handle, uint32_t slot) const override;
  void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) override;
  void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) override;
  void GenerateMipmaps(TextureGPUHandle handle) override;

  // ---- 模型操作 ----
  ModelGPUHandle CreateModel(const ModelSourceData &data) override;
  void DestroyModel(ModelGPUHandle model) override;
  void BindMesh(std::shared_ptr<ModelGPUHandle> modelHandle,
                MeshSection meshSection) const override;
  void DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const override;

 private:
  // ---- 事件响应函数 ----
  void OnModelLoaded(ModelLoadEvent &e) override;
  void OnTextureLoaded(TextureLoadEvent &e) override;

  // ---- 辅助方法 ----
  GLenum TranslateTextureFormat(TextureFormat format);
  GLenum ConvertWrapMode(TextureWrapMode mode) const;
  void ConvertFilterMode(TextureFilterMode mode, GLenum &outMinFilter, GLenum &outMagFilter) const;
  void SetVertexAttributes(const VertexLayout &layout);

  // 资源追踪（用于调试和泄漏检测）
  std::unordered_set<GLuint> activeTextures_;
  std::unordered_set<GLuint> activeModelsVAO_, activeModelsVBO_, activeModelsEBO_;

  // 日志系统
  Logger m_Logger;
};
};  // namespace mite

#endif
