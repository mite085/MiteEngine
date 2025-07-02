#ifndef MITE_RENDERER_TEXTURE
#define MITE_RENDERER_TEXTURE

#include "asset_type.h"
#include "render_device.h"

namespace mite {
class Texture {
 public:
  // 从TextureAsset创建纹理（由AssetManager调用）
  explicit Texture(TextureGPUHandle handle, TextureWrapMode wrap, TextureFilterMode filter);
  ~Texture();

  // 绑定到指定纹理单元
  void Bind(uint32_t slot) const;

  // 获取元数据
  const TextureMetadata &GetMetadata() const
  {
    return m_Metadata;
  }

  //// 获取API句柄（供其他渲染操作使用）
  //TextureGPUHandle GetHandle() const
  //{
  //  return m_Handle;
  //}

 private:
  TextureMetadata m_Metadata;
  //TextureGPUHandle m_Handle;
};

};  // namespace mite

#endif
