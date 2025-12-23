#ifndef MITE_FRAMEBUFFER_TYPE
#define MITE_FRAMEBUFFER_TYPE

#include "basic_data/runtime_texture.h"
#include "handle_type.h"

namespace mite {
// ------------------------ 帧缓冲相关 ------------------------

// 帧缓冲附件规格结构体（直接使用RuntimeTextureType来明确指定附件的具体用途）
// Depth和Stencil应当手动明确Format，此处不做约束
struct FrameBufferAttachmentSpec {
  RuntimeTextureType type = RuntimeTextureType::RenderTarget;
  TextureFormat internalFormat = TextureFormat::RGB8;        // 内部格式
  TextureTarget internalTarget = TextureTarget::TEXTURE_2D;  // 目标纹理类型
  bool generateMipmaps = false;  // 是否生成mipmaps
  bool isArrayTexture = false;   // 是否为数组纹理
  bool isCubeMap = false;        // 是否为立方体贴图
  uint32_t arrayLayers = 1;      // 数组层数（如果是数组纹理）
};

// 帧缓冲规格结构体
struct FrameBufferSpec {
  uint32_t width = 1;   // 默认宽度1，强制要求手动指定
  uint32_t height = 1;  // 默认高度1，强制要求手动指定
  std::vector<FrameBufferAttachmentSpec> attachments;  // 附件列表
  uint32_t samples = 1;  // 多重采样数(默认为1，即不启用)
};
};  // namespace mite

#endif
