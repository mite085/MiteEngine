#ifndef MITE_RENDER_DEVICE
#define MITE_RENDER_DEVICE

#include "asset_type.h"

namespace mite {
/**
 * 渲染设备抽象接口（由Renderer模块实现OpenGLDevice）
 * 
 * 单例模式，确保同一时间仅一个Device
 */
class IRenderDevice {
 public:
  virtual ~IRenderDevice() = default;

  // 纹理操作
  virtual TextureGPUHandle CreateTexture(const TextureAsset &texture) = 0;
  virtual void DestroyTexture(TextureGPUHandle handle) = 0;
  virtual void BindTexture(TextureGPUHandle handle, uint32_t slot) const = 0;
  virtual void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) = 0;
  virtual void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) = 0;

  // 模型/网格体操作
  virtual ModelGPUHandle CreateModel(const ModelAsset &meta) = 0;
  virtual void DestroyModel(ModelGPUHandle handle) = 0;
  virtual void BindMesh(MeshGPUHandle handle) const = 0;
  virtual void DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const = 0;

  // 静态当前设备管理
  static IRenderDevice &Current();
  static void SetCurrent(IRenderDevice *device);
};
};  // namespace mite

#endif
