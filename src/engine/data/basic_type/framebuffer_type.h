#ifndef MITE_FRAMEBUFFER_TYPE
#define MITE_FRAMEBUFFER_TYPE

#include "handle_type.h"
#include "basic_data/runtime_texture.h"

namespace mite {
// ------------------------ 帧缓冲相关 ------------------------

// 帧缓冲附件规格结构体（直接使用RuntimeTextureType来明确指定附件的具体用途）
// Depth和Stencil应当手动明确Format，此处不做约束
struct FrameBufferAttachmentSpec {
  RuntimeTexture::RuntimeTextureType type = RuntimeTexture::RuntimeTextureType::RenderTarget;
  TextureFormat internalFormat = TextureFormat::RGB8;  // 内部格式
  bool generateMipmaps = false;                        // 是否生成mipmaps
};

// 帧缓冲规格结构体
struct FrameBufferSpec {
  uint32_t width = 1;                                  // 默认宽度1，强制要求手动指定
  uint32_t height = 1;                                 // 默认高度1，强制要求手动指定
  std::vector<FrameBufferAttachmentSpec> attachments;  // 附件列表
  uint32_t samples = 1;  // 多重采样数(默认为1，即不启用)
};
};  // namespace mite

#endif
