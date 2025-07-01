#ifndef MITE_OPENGL_DEVICE
#define MITE_OPENGL_DEVICE

#include "render_device.h"
#include "glad.h"
#include "GLFW/glfw3.h"// 必须在GLAD加载库之后

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
  TextureGPUHandle CreateTexture(const TextureMetadata &meta, const void *data) override;
  void DestroyTexture(TextureGPUHandle handle) override;

  // ---- 模型操作 ----
  ModelGPUHandle CreateModel(const ModelMetadata &meta) override;
  void DestroyModel(ModelGPUHandle handle) override;

  // ---- 辅助方法 ----
  static GLenum TranslateTextureFormat(TextureFormat format);

 private:
  // 资源追踪（用于调试和泄漏检测）
  std::unordered_map<GLuint, TextureMetadata> activeTextures_;
  std::unordered_map<GLuint, ModelMetadata> activeModels_;
};
};

#endif
