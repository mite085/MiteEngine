#include "texture.h"

namespace mite {
Texture::Texture(const TextureGPUHandle &handle) : handle_(handle) {}

void Texture::Bind(uint32_t slot) const
{
  // IRenderDevice::Current().BindTexture(handle_, slot);

  TextureBindEvent event(handle_, slot);
  EventBus::Get().Post(event);
}

void Texture::SetWrapMode(TextureWrapMode mode)
{
  // IRenderDevice::Current().SetTextureWrapMode(handle_, mode);

  TextureWrapModeEvent event(handle_, mode);
  EventBus::Get().Post(event);
}

void Texture::SetFilterMode(TextureFilterMode mode)
{
  // IRenderDevice::Current().SetTextureFilterMode(handle_, mode);

  TextureFilterModeEvent event(handle_, mode);
  EventBus::Get().Post(event);
}

void Texture::GenerateMipmaps()
{
  // IRenderDevice::Current().GenerateMipmaps(handle_);

  TextureGenerateMipmapsEvent event(handle_);
  EventBus::Get().Post(event);
}
};  // namespace mite