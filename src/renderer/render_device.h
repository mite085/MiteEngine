#ifndef MITE_RENDER_DEVICE
#define MITE_RENDER_DEVICE

#include "handle_types.h"
#include "model_loader.h"
#include "texture_loader.h"

namespace mite {

/**
 * 渲染设备抽象接口
 *
 * 单例模式：
 * 使用单例模式的目的是方便Texture和Mesh每次创建时可以不通过对IRenderDevice的
 * 依赖注入，且可独立实现Draw方法，确保代码的简洁性。
 *
 *
 * 但存在风险：
 * 1. 如果渲染指令需在多个线程提交（如渲染线程 vs. 资源加载线程），
 *	  单例的全局锁可能成为性能瓶颈。此时需设计无锁队列或线程局部存储（TLS）。
 * 2. 单例的 IRenderDevice 会阻碍单元测试中对渲染接口的模拟（Mocking）。
 *    依赖注入更利于隔离测试。
 * 3. 即使当前无需多渲染器，未来可能支持多视口、多GPU或离线渲染。单例会限制架构灵活性。
 */
class IRenderDevice {
 public:
  virtual ~IRenderDevice() = default;

  // 纹理操作
  virtual TextureGPUHandle CreateTexture(const TextureSourceData &data) = 0;
  virtual void DestroyTexture(TextureGPUHandle handle) = 0;
  virtual void BindTexture(TextureGPUHandle handle, uint32_t slot) const = 0;
  virtual void SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode) = 0;
  virtual void SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode) = 0;
  virtual void GenerateMipmaps(TextureGPUHandle handle) = 0;

  // 模型/网格体操作
  virtual ModelGPUHandle CreateModel(const ModelSourceData &data) = 0;
  virtual void DestroyModel(ModelGPUHandle handle) = 0;
  virtual void BindMesh(MeshGPUHandle handle) const = 0;
  virtual void DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const = 0;

  // 静态当前设备管理
  static IRenderDevice &Current();
  static void SetCurrent(std::unique_ptr<IRenderDevice> device);

 private:
  // 私有构造函数
  IRenderDevice();

  // ---- 事件响应函数 ----
  virtual void OnModelLoaded(ModelLoadEvent &e) = 0;
  virtual void OnTextureLoaded(TextureLoadEvent &e) = 0;
  virtual void OnModelDrawed(ModelDrawEvent &e) = 0;
  virtual void OnMeshDrawed(MeshDrawEvent &e) = 0;

};
};  // namespace mite

#endif
