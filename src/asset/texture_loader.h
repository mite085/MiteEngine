#ifndef MITE_ASSET_TEXTURE_LOADER
#define MITE_ASSET_TEXTURE_LOADER

#include "asset_type.h"

namespace mite {
class TextureLoader {
 public:
  /**
   * 加载标准LDR纹理（PNG/JPG/BMP等）
   * @param path 纹理文件路径（支持相对/绝对路径）
   * @param desiredChannels 强制转换的通道数（0=保持原样，3=RGB，4=RGBA）
   * @param flipVertical 是否垂直翻转图像（适配OpenGL坐标系）
   * @return 纹理元数据 + 原始像素数据（需由调用者上传至GPU）
   * @throws std::runtime_error 当文件加载失败时抛出异常
   */
  static std::shared_ptr<TextureAsset> LoadTextureData(const std::string &path,
                                                       int desiredChannels = 4,
                                                       bool flipVertical = true);

  /**
   * 释放stb_image分配的像素内存
   * @param data stb_image返回的像素数据指针
   */
  static void FreeTextureData(void *data);
};
};  // namespace mite

#endif
