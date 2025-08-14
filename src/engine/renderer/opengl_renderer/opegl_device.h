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
  TextureGPUHandle CreateTexture(std::shared_ptr<TextureSourceData> data) override;
  void DestroyTexture(TextureGPUHandle handle) override;
  void BindTexture(TextureGPUHandle handle, uint32_t slot) const override;
  void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) override;
  void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) override;
  void GenerateMipmaps(TextureGPUHandle handle) override;

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
  void DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const override;

 private:
  // ---- 事件响应函数 ----
  bool OnModelLoaded(ModelLoadEvent &e) override;
  bool OnTextureLoaded(TextureLoadEvent &e) override;

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
