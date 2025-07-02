#include "texture.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后

namespace mite {
Texture::Texture(const TextureAsset &asset)
    : m_Metadata(asset.metadata)
{
  // 实际创建由IRenderDevice完成
}

Texture::~Texture()
{
  // 析构时无需操作，由AssetManager统一管理生命周期
}

void Texture::Bind(uint32_t slot) const
{
  //glActiveTexture(GL_TEXTURE0 + slot);
  //glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(m_Handle.apiHandle));
}
};  // namespace mite