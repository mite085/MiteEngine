#include "texture.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后

namespace mite {
Texture::Texture(const TextureGPUHandle &handle, const TextureMetadata &meta)
    : handle_(handle), meta_(meta)
{
  // 初始状态设置
  SetWrapMode(TextureWrapMode::Repeat);
  SetFilterMode(TextureFilterMode::Linear);
}

void Texture::Bind(uint32_t slot) const
{
  IRenderDevice::Current().BindTexture(handle_, slot);
}

void Texture::SetWrapMode(TextureWrapMode mode)
{
  IRenderDevice::Current().SetTextureWrapMode(handle_, mode);
}

void Texture::SetFilterMode(TextureFilterMode mode) {
  IRenderDevice::Current().SetTextureFilterMode(handle_, mode);
}

//void Texture::GenerateMipmaps()
//{
//  if (meta_.format == TextureFormat::RGB16F || meta_.format == TextureFormat::RGBA16F) {
//    IRenderDevice::Current().GenerateMipmaps(handle_);
//  }
//}
};  // namespace mite