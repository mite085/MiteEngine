#ifndef MITE_KTX2_LOADER_H
#define MITE_KTX2_LOADER_H

#include "asset_cache.h"
#include "basic_event/asset_event.h"
#include "basic_type/texture_type.h"

namespace mite {

/**
 * KTX2纹理加载器 - 专门处理KTX2格式纹理
 * 支持特性：
 * - 压缩纹理格式
 * - 多级mipmaps
 * - 立方体贴图
 * - 纹理数组
 */
class KTX2Loader {
 public:
  static TextureAssetID LoadKTX2Texture(TextureCache &cache, const std::string &path);

  static bool IsKTX2File(const std::string &path);

 private:
  static TextureAssetID LoadKTX2TextureInternal(TextureCache &cache, const std::string &path);

  static TextureFormat KTXFormatToTextureFormat(uint32_t ktxFormat);
  static TextureTarget KTXTypeToTextureTarget(uint32_t numDimensions,
                                              uint32_t numFaces,
                                              uint32_t numLayers);
  static void SetupKTXSamplingParams(TextureMetadata &metadata, bool isCompressed, bool isCubeMap);
  static uint32_t GetBlockSizeForFormat(uint32_t glInternalFormat);
  static bool IsSRGBFormat(uint32_t glInternalFormat);
};

}  // namespace mite

#endif
