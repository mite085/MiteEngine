#ifndef MITE_RENDER_DEVICE
#define MITE_RENDER_DEVICE

#include "asset_type.h"

namespace mite {
/**
 * 渲染设备抽象接口（由Renderer模块实现OpenGLDevice）
 */
class IRenderDevice {
 public:
  virtual ~IRenderDevice() = default;

  // 纹理操作
  virtual TextureGPUHandle CreateTexture(const TextureAsset &texture) = 0;
  virtual void DestroyTexture(TextureGPUHandle handle) = 0;

  // 模型操作
  virtual ModelGPUHandle CreateModel(const ModelAsset &meta) = 0;
  virtual void DestroyModel(ModelGPUHandle handle) = 0;
};
};

#endif
