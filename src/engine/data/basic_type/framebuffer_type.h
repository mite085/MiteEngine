#ifndef MITE_FRAMEBUFFER_TYPE
#define MITE_FRAMEBUFFER_TYPE

#include "headers/headers.h"

namespace mite {
// ------------------------ 帧缓冲相关 ------------------------

// 帧缓冲附件类型枚举
enum class FrameBufferAttachmentType {
  Color = 0,    // 颜色附件
  Depth,        // 深度附件
  Stencil,      // 模板附件
  DepthStencil  // 深度模板组合附件
};

// 帧缓冲附件规格结构体
// 若为Depth，无需定义内部格式和数据类型，FrameBuffer::Invalidate()
// 检测到Type为depth，自动忽略格式，按照GL_DEPTH_COMPONENT24
struct FrameBufferAttachmentSpec {
  FrameBufferAttachmentType type = FrameBufferAttachmentType::Color;
  GLenum internalFormat = GL_RGBA8;  // 内部格式
  bool generateMipmaps = false;      // 是否生成mipmaps
};

// 帧缓冲规格结构体
struct FrameBufferSpec {
  uint32_t width = 1280;                               // 默认宽度
  uint32_t height = 720;                               // 默认高度
  std::vector<FrameBufferAttachmentSpec> attachments;  // 附件列表
  uint32_t samples = 1;  // 多重采样数(默认为1，即不启用)
};
};  // namespace mite

#endif
