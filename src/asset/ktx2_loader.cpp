#include "ktx2_loader.h"
#include <ktx.h>

namespace mite {

TextureAssetID KTX2Loader::LoadKTX2Texture(TextureCache &cache, const std::string &path)
{
  if (path.empty()) {
    LOG_ERROR("KTX2 texture path is empty");
    return TextureAssetID();
  }

  // 检查缓存
  TextureAssetID existingId(UUIDGenerator::Generate(path.c_str()));
  auto texture = cache.Get(existingId);

  if (texture) {
    LOG_INFO("KTX2 texture already cached: " + path);
    return existingId;
  }

  return LoadKTX2TextureInternal(cache, path);
}

bool KTX2Loader::IsKTX2File(const std::string &path)
{
  if (path.length() < 5)
    return false;

  std::string extension = path.substr(path.length() - 5);
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  return extension == ".ktx2";
}
TextureAssetID KTX2Loader::LoadKTX2TextureInternal(TextureCache &cache, const std::string &path)
{
  ktxTexture *ktxTexture = nullptr;
  KTX_error_code result = ktxTexture_CreateFromNamedFile(
      path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

  if (result != KTX_SUCCESS) {
    LOG_ERROR("Failed to load KTX2 texture: " + path + ", error: " + std::to_string(result));
    return TextureAssetID();
  }

  // 创建纹理资产
  auto textureAsset = std::make_shared<TextureAsset>();
  textureAsset->id = TextureAssetID{UUIDGenerator::Generate(path.c_str())};

  // 填充元数据
  textureAsset->metadata.sourcePath = path;
  textureAsset->metadata.originalWidth = ktxTexture->baseWidth;
  textureAsset->metadata.originalHeight = ktxTexture->baseHeight;
  textureAsset->metadata.target = KTXTypeToTextureTarget(
      ktxTexture->numDimensions, ktxTexture->numFaces, ktxTexture->numLayers);

  // 获取GL格式信息
  ktx_uint32_t glInternalFormat = GL_RGBA8;
  ktx_uint32_t glFormat = 0;
  ktx_uint32_t glType = 0;
  ktx_uint32_t glBaseInternalFormat = 0;

  // 转换KTX格式到引擎格式
  textureAsset->metadata.sourceFormat = KTXFormatToTextureFormat(glInternalFormat);

  // 设置KTX特定的采样参数
  bool isCompressed = ktxTexture->isCompressed;
  bool isCubeMap = ktxTexture->isCubemap;
  SetupKTXSamplingParams(textureAsset->metadata, isCompressed, isCubeMap);

  // 准备KTX2特定的源数据
  TextureSourceData sourceData;
  sourceData.width = ktxTexture->baseWidth;
  sourceData.height = ktxTexture->baseHeight;
  sourceData.depth = ktxTexture->baseDepth;
  sourceData.format = textureAsset->metadata.sourceFormat;
  sourceData.target = textureAsset->metadata.target;
  sourceData.isCompressed = isCompressed;
  sourceData.isCubeMap = isCubeMap;
  sourceData.arrayLayers = ktxTexture->numLayers;

  // 复制采样参数
  sourceData.wrapModeS = textureAsset->metadata.wrapModeS;
  sourceData.wrapModeT = textureAsset->metadata.wrapModeT;
  sourceData.minFilter = textureAsset->metadata.minFilter;
  sourceData.magFilter = textureAsset->metadata.magFilter;
  sourceData.generateMipmaps = textureAsset->metadata.generateMipmaps;

  // 提取所有mipmap级别的数据
  for (ktx_uint32_t level = 0; level < ktxTexture->numLevels; level++) {
    ktx_size_t offset;
    ktx_uint32_t layer = 0, faceSlice = 0;

    // 获取当前mip级别的数据
    result = ktxTexture_GetImageOffset(ktxTexture, level, layer, faceSlice, &offset);
    if (result != KTX_SUCCESS) {
      LOG_WARN("Failed to get image offset for level " + std::to_string(level));
      continue;
    }

    // 计算当前mip级别的大小
    ktx_uint32_t levelWidth = std::max(1u, ktxTexture->baseWidth >> level);
    ktx_uint32_t levelHeight = std::max(1u, ktxTexture->baseHeight >> level);
    ktx_size_t imageSize = ktxTexture_GetImageSize(ktxTexture, level);

    if (imageSize == 0) {
      LOG_WARN("Zero image size for level " + std::to_string(level));
      continue;
    }

    // 提取数据
    ktx_uint8_t *imageData = ktxTexture_GetData(ktxTexture) + offset;
    std::vector<uint8_t> mipData(imageSize);
    memcpy(mipData.data(), imageData, imageSize);
    sourceData.mipLevels.push_back(std::move(mipData));

    LOG_DEBUG("Extracted mip level " + std::to_string(level) + " (" + std::to_string(levelWidth) +
              "x" + std::to_string(levelHeight) + ", size: " + std::to_string(imageSize) +
              " bytes)");
  }

  sourceData.existingMipLevels = static_cast<uint32_t>(sourceData.mipLevels.size());

  // 如果没有提取到mip数据，使用主数据
  if (sourceData.mipLevels.empty() && ktxTexture->pData) {
    std::vector<uint8_t> mainData(ktxTexture->dataSize);
    memcpy(mainData.data(), ktxTexture->pData, ktxTexture->dataSize);
    sourceData.mipLevels.push_back(std::move(mainData));
    sourceData.existingMipLevels = 1;
    LOG_DEBUG("Using main texture data, size: " + std::to_string(ktxTexture->dataSize) + " bytes");
  }

  // 设置压缩信息（如果是压缩格式）
  if (sourceData.isCompressed) {
    sourceData.compressionInfo.blockSize = GetBlockSizeForFormat(glInternalFormat);
    sourceData.compressionInfo.internalFormat = glInternalFormat;
    sourceData.compressionInfo.isSRGB = IsSRGBFormat(glInternalFormat);
  }

  // 发布纹理加载事件
  TextureLoadEvent event(std::make_shared<TextureSourceData>(std::move(sourceData)), textureAsset);
  EventBus::Publish<TextureLoadEvent>(event);

  // 存入缓存
  if (cache.Store(textureAsset)) {
    LOG_INFO("Successfully loaded KTX2 texture: " + path + " (" +
             std::to_string(ktxTexture->baseWidth) + "x" + std::to_string(ktxTexture->baseHeight) +
             ", mips: " + std::to_string(ktxTexture->numLevels) +
             ", faces: " + std::to_string(ktxTexture->numFaces) +
             ", compressed: " + (isCompressed ? "yes" : "no") + ")");
    ktxTexture_Destroy(ktxTexture);
    return textureAsset->id;
  }

  ktxTexture_Destroy(ktxTexture);
  LOG_ERROR("Failed to store KTX2 texture in cache: " + path);
  return TextureAssetID();
}
TextureFormat KTX2Loader::KTXFormatToTextureFormat(uint32_t glInternalFormat)
{
  // 常见GL内部格式到引擎格式的映射
  switch (glInternalFormat) {
    // 未压缩格式
    case GL_R8:
      return TextureFormat::R8;
    case GL_RG8:
      return TextureFormat::RG8;
    case GL_RGB8:
      return TextureFormat::RGB8;
    case GL_RGBA8:
      return TextureFormat::RGBA8;
    case GL_SRGB8:
      return TextureFormat::SRGB8;
    case GL_SRGB8_ALPHA8:
      return TextureFormat::SRGB8_ALPHA8;
    case GL_R16F:
      return TextureFormat::R16F;
    case GL_RG16F:
      return TextureFormat::RG16F;
    case GL_RGB16F:
      return TextureFormat::RGB16F;
    case GL_RGBA16F:
      return TextureFormat::RGBA16F;
    case GL_R32F:
      return TextureFormat::R32F;
    case GL_RG32F:
      return TextureFormat::RG32F;
    case GL_RGB32F:
      return TextureFormat::RGB32F;
    case GL_RGBA32F:
      return TextureFormat::RGBA32F;

    // 深度/模板格式
    case GL_DEPTH_COMPONENT16:
      return TextureFormat::DEPTH_COMPONENT16;
    case GL_DEPTH_COMPONENT24:
      return TextureFormat::DEPTH_COMPONENT24;
    case GL_DEPTH_COMPONENT32:
      return TextureFormat::DEPTH_COMPONENT32;
    case GL_DEPTH24_STENCIL8:
      return TextureFormat::DEPTH24_STENCIL8;
    case GL_STENCIL_INDEX8:
      return TextureFormat::STENCIL_INDEX8;

    // 压缩格式 - 返回最接近的未压缩格式，实际渲染器需要特殊处理
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
      return TextureFormat::RGBA8;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      return TextureFormat::RGB8;

    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      return TextureFormat::RGB16F;

    default:
      LOG_WARN("Unsupported KTX format: 0x" + std::to_string(glInternalFormat) + ", using RGBA8");
      return TextureFormat::RGBA8;
  }
}
TextureTarget KTX2Loader::KTXTypeToTextureTarget(uint32_t numDimensions,
                                                 uint32_t numFaces,
                                                 uint32_t numLayers)
{
  if (numFaces == 6) {
    if (numLayers > 1) {
      return TextureTarget::TEXTURE_CUBE_MAP_ARRAY;
    }
    return TextureTarget::TEXTURE_CUBE_MAP;
  }

  switch (numDimensions) {
    case 1:
      return TextureTarget::TEXTURE_2D;  // 1D纹理在OpenGL中不常用
    case 2:
      if (numLayers > 1) {
        return TextureTarget::TEXTURE_2D_ARRAY;
      }
      return TextureTarget::TEXTURE_2D;
    case 3:
      return TextureTarget::TEXTURE_3D;
    default:
      LOG_WARN("Unknown texture dimensions: " + std::to_string(numDimensions));
      return TextureTarget::TEXTURE_2D;
  }
}
void KTX2Loader::SetupKTXSamplingParams(TextureMetadata &metadata,
                                        bool isCompressed,
                                        bool isCubeMap)
{
  if (isCubeMap) {
    // 立方体贴图使用ClampToEdge避免接缝
    metadata.wrapModeS = TextureWrapMode::ClampToEdge;
    metadata.wrapModeT = TextureWrapMode::ClampToEdge;
  }
  else {
    metadata.wrapModeS = TextureWrapMode::Repeat;
    metadata.wrapModeT = TextureWrapMode::Repeat;
  }

  if (isCompressed) {
    // 压缩纹理通常已经包含mipmaps，不需要生成
    metadata.minFilter = TextureFilterMode::LinearMipmapLinear;
    metadata.magFilter = TextureFilterMode::Linear;
    metadata.generateMipmaps = false;
  }
  else {
    metadata.minFilter = TextureFilterMode::LinearMipmapLinear;
    metadata.magFilter = TextureFilterMode::Linear;
    metadata.generateMipmaps = true;
  }
}
uint32_t KTX2Loader::GetBlockSizeForFormat(uint32_t glInternalFormat)
{
  // 根据压缩格式返回块大小
  switch (glInternalFormat) {
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      return 8;  // DXT1: 8 bytes per 4x4 block

    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      return 16;  // DXT3/DXT5: 16 bytes per 4x4 block

    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
      return 16;  // ASTC 4x4: 16 bytes per 4x4 block
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
      return 16;  // ASTC 8x8: 16 bytes per 8x8 block

    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      return 16;  // BPTC: 16 bytes per 4x4 block

    default:
      return 0;  // 未压缩格式
  }
}
bool KTX2Loader::IsSRGBFormat(uint32_t glInternalFormat)
{
  switch (glInternalFormat) {
    case GL_SRGB8:
    case GL_SRGB8_ALPHA8:
    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      return true;
    default:
      return false;
  }
}
}  // namespace mite