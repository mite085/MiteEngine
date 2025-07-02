#ifndef MITE_RENDERER_TEXTURE
#define MITE_RENDERER_TEXTURE

#include "asset_type.h"
#include "render_device.h"

namespace mite {
/**
 * GPU纹理运行时封装
 * 职责：
 * - 维护纹理采样状态（Wrap/Filter等）
 * - 提供类型安全的绑定接口
 * - 不管理生命周期（由Renderer负责）
 */
class Texture {
 public:
  Texture(const TextureGPUHandle &handle, const TextureMetadata &meta);

  // ---- 核心接口 ----
  void Bind(uint32_t slot) const;
  //void Unbind() const;

  // ---- 状态设置 ----
  void SetWrapMode(TextureWrapMode mode);
  void SetFilterMode(TextureFilterMode mode);
  //void GenerateMipmaps();

  // ---- 元数据访问 ----
  TextureGPUHandle GetHandle() const
  {
    return handle_;
  }
  glm::ivec2 GetSize() const
  {
    return {meta_.width, meta_.height};
  }
  bool IsHDR() const
  {
    return meta_.isHDR;
  }

 private:
  TextureGPUHandle handle_;
  TextureMetadata meta_;
};

};  // namespace mite

#endif
