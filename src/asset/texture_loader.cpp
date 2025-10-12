#include "texture_loader.h"
#include <assimp/MemoryIOWrapper.h>
#include <assimp/scene.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace mite {
IBLConfig TextureLoader::s_IBLConfig;
void TextureLoader::SetIBLConfig(const IBLConfig &config)
{
  s_IBLConfig = config;
  std::filesystem::create_directories(s_IBLConfig.cacheDirectory);
  LOG_INFO("IBL config updated, cache directory: " + s_IBLConfig.cacheDirectory);
}
const IBLConfig &TextureLoader::GetIBLConfig()
{
  return s_IBLConfig;
}
bool TextureLoader::IsKTX2File(const std::string &path)
{
  return KTX2Loader::IsKTX2File(path);
}
bool TextureLoader::IsHDRFile(const std::string &path)
{
  if (path.length() < 4)
    return false;

  std::string extension = path.substr(path.length() - 4);
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  return extension == ".hdr" || extension == ".exr";
}

TextureAssetID TextureLoader::LoadTexture(TextureCache &cache,
                                          const std::string &path,
                                          int desiredChannels,
                                          bool flipVertical)
{
  // 检查路径有效性
  if (path.empty()) {
    LOG_ERROR("Texture path is empty");
    return TextureAssetID();  // 返回无效ID
  }
  // 嵌入式纹理需要调用LoadEmbeddedTexture，输入ModelPath。这个函数无法处理
  if (IsEmbeddedTexturePath(path)) {
    LOG_ERROR("External texture path should not be embedded format: " + path);
    return TextureAssetID();  // 返回无效ID
  }

  // 检查缓存中是否已存在相同路径的纹理
  TextureAssetID existingId = FindTextureByPath(cache, path);
  if (existingId.IsValid()) {
    LOG_INFO("Texture already cached: " + path);
    return existingId;
  }

  // 格式分发
  if (IsKTX2File(path)) {
    return KTX2Loader::LoadKTX2Texture(cache, path);
  }
  else if (IsHDRFile(path)) {
    return LoadHDRTexture(cache, path, flipVertical);
  }
  else {
    // 调用内部加载实现，嵌入式数据为空
    return LoadTextureInternal(cache, path, {}, desiredChannels, flipVertical);
  }
}

TextureAssetID TextureLoader::LoadEmbeddedTexture(TextureCache &cache,
                                                  const std::string &embeddedId,
                                                  const std::string &modelPath,
                                                  const aiTexture *aiTexture,
                                                  int desiredChannels,
                                                  bool flipVertical)
{
  // 参数检查
  if (!aiTexture) {
    LOG_ERROR("Null aiTexture provided for embedded texture");
    return TextureAssetID();
  }

  if (embeddedId.empty() || embeddedId[0] != '*') {
    LOG_ERROR("Invalid embedded texture ID: " + embeddedId);
    return TextureAssetID();
  }

  // 生成唯一的纹理路径标识：模型路径 + 嵌入式ID
  std::string texturePath = modelPath + embeddedId;

  // 检查缓存中是否已存在
  TextureAssetID existingId = FindTextureByPath(cache, texturePath);
  if (existingId.IsValid()) {
    LOG_INFO("Embedded texture already cached: " + texturePath);
    return existingId;
  }

  // 从Assimp纹理提取嵌入式数据
  std::vector<uint8_t> embeddedData = ExtractEmbeddedData(aiTexture);

  if (embeddedData.empty()) {
    LOG_ERROR("Failed to extract embedded data from aiTexture: " + texturePath);
    return TextureAssetID();
  }

  // 调用内部加载实现
  return LoadTextureInternal(cache, texturePath, embeddedData, desiredChannels, flipVertical);
}
TextureAssetID TextureLoader::LoadHDRTexture(TextureCache &cache,
                                             const std::string &path,
                                             bool flipVertical)
{
  if (path.empty()) {
    LOG_ERROR("HDR texture path is empty");
    return TextureAssetID();
  }

  // 检查缓存
  TextureAssetID existingId = FindTextureByPath(cache, path);
  if (existingId.IsValid()) {
    LOG_INFO("HDR texture already cached: " + path);
    return existingId;
  }

  return LoadHDRTextureInternal(cache, path, flipVertical);
}

TextureAssetID TextureLoader::LoadTextureInternal(TextureCache &cache,
                                                  const std::string &path,
                                                  const std::vector<uint8_t> &embeddedData,
                                                  int desiredChannels,
                                                  bool flipVertical)
{
  // 设置STB图像加载配置
  stbi_set_flip_vertically_on_load(flipVertical);

  // 准备加载变量
  int width = 0, height = 0, channels = 0;
  uint8_t *pixelData = nullptr;

  // 区分外部文件和嵌入式数据加载
  if (embeddedData.empty()) {
    // 外部文件加载
    pixelData = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
  }
  else {
    // 嵌入式数据加载
    pixelData = stbi_load_from_memory(embeddedData.data(),
                                      static_cast<int>(embeddedData.size()),
                                      &width,
                                      &height,
                                      &channels,
                                      desiredChannels);
  }
  // 检查加载结果
  if (!pixelData) {
    LOG_ERROR("Failed to load texture: " + path + ", reason: " + stbi_failure_reason());
    return TextureAssetID();
  }

  // 计算实际通道数
  int actualChannels = (desiredChannels > 0) ? desiredChannels : channels;

  // 创建纹理资产
  auto textureAsset = std::make_shared<TextureAsset>();

  // 设置资产ID（基于路径生成）
  textureAsset->id = TextureAssetID{UUIDGenerator::Generate(path.c_str())};

  // 填充元数据
  textureAsset->metadata.sourcePath = path;
  textureAsset->metadata.originalWidth = static_cast<uint32_t>(width);
  textureAsset->metadata.originalHeight = static_cast<uint32_t>(height);
  textureAsset->metadata.sourceFormat = DetermineLDRTextureFormat(actualChannels);
  textureAsset->metadata.target = TextureTarget::TEXTURE_2D;

  // 设置默认采样参数
  SetupDefaultSamplingParams(textureAsset->metadata);

  // 创建像素数据向量并转移所有权
  size_t dataSize = width * height * actualChannels;
  std::vector<uint8_t> pixelVector(pixelData, pixelData + dataSize);

  // 释放STB分配的内存
  stbi_image_free(pixelData);

  // 生成TextureSourceData供Renderer使用
  TextureSourceData sourceData = textureAsset->metadata.generateSourceData(std::move(pixelVector));

  // 发布纹理加载事件，委托Renderer创建GPU资源
  TextureLoadEvent event(std::make_shared<TextureSourceData>(std::move(sourceData)), textureAsset);
  EventBus::Publish<TextureLoadEvent>(event);

  // 将纹理资产存入缓存
  if (cache.Store(textureAsset)) {
    TextureAssetID textureId = textureAsset->GetID();
    LOG_INFO("Successfully loaded and cached texture: " + path + " (" + std::to_string(width) +
             "x" + std::to_string(height) + ", channels: " + std::to_string(actualChannels) + ")");
    return textureId;
  }
  else {
    LOG_ERROR("Failed to store texture in cache: " + path);
    return TextureAssetID();
  }
}
TextureAssetID TextureLoader::LoadHDRTextureInternal(TextureCache &cache,
                                                     const std::string &path,
                                                     bool flipVertical)
{
  stbi_set_flip_vertically_on_load(flipVertical);

  int width = 0, height = 0, channels = 0;
  float *pixelData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

  if (!pixelData) {
    LOG_ERROR("Failed to load HDR texture: " + path + ", reason: " + stbi_failure_reason());
    return TextureAssetID();
  }

  // 创建纹理资产
  auto textureAsset = std::make_shared<TextureAsset>();
  textureAsset->id = TextureAssetID{UUIDGenerator::Generate(path.c_str())};

  // 填充元数据 - 使用浮点格式
  textureAsset->metadata.sourcePath = path;
  textureAsset->metadata.originalWidth = static_cast<uint32_t>(width);
  textureAsset->metadata.originalHeight = static_cast<uint32_t>(height);
  textureAsset->metadata.sourceFormat = DetermineHDRTextureFormat(channels);
  textureAsset->metadata.target = TextureTarget::TEXTURE_2D;

  // 设置HDR纹理的默认采样参数
  SetupHDRSamplingParams(textureAsset->metadata);

  // 转换数据为字节流（保持浮点精度）
  size_t dataSize = width * height * channels * sizeof(float);
  std::vector<uint8_t> pixelVector(dataSize);
  memcpy(pixelVector.data(), pixelData, dataSize);

  stbi_image_free(pixelData);

  // 生成TextureSourceData
  TextureSourceData sourceData = textureAsset->metadata.generateSourceData(std::move(pixelVector));

  // 发布加载事件
  TextureLoadEvent event(std::make_shared<TextureSourceData>(std::move(sourceData)), textureAsset);
  EventBus::Publish<TextureLoadEvent>(event);

  if (cache.Store(textureAsset)) {
    LOG_INFO("Successfully loaded HDR texture: " + path + " (" + std::to_string(width) + "x" +
             std::to_string(height) + ", HDR channels: " + std::to_string(channels) + ")");
    return textureAsset->id;
  }

  return TextureAssetID();
}

TextureAssetID TextureLoader::LoadEnvironmentTextureInternal(TextureCache &cache,
                                                             const std::string &path,
                                                             bool flipVertical)
{
  // 首先加载HDR纹理
  TextureAssetID hdrTextureId = LoadHDRTextureInternal(cache, path, flipVertical);
  if (!hdrTextureId.IsValid()) {
    LOG_ERROR("Failed to load HDR texture for environment: " + path);
    return TextureAssetID();
  }

  // 自动生成IBL纹理
  if (s_IBLConfig.autoGenerateIBL) {
    std::string environmentId = IBLGenerator::GenerateEnvironmentId(path);
    GenerateAndLoadIBLTextures(cache, path, environmentId);
  }

  return hdrTextureId;
}

std::vector<uint8_t> TextureLoader::ExtractEmbeddedData(const aiTexture *aiTexture)
{
  std::vector<uint8_t> data;
  if (!aiTexture)
    return data;
  if (aiTexture->mHeight == 0) {
    // 使用Assimp的内存IO接口
    Assimp::MemoryIOStream stream(reinterpret_cast<const uint8_t *>(aiTexture->pcData),
                                  aiTexture->mWidth);

    // 读取直到EOF
    std::vector<uint8_t> buffer;
    const size_t chunkSize = 4096;
    uint8_t chunk[chunkSize];

    while (true) {
      size_t read = stream.Read(chunk, 1, chunkSize);
      if (read == 0)
        break;
      buffer.insert(buffer.end(), chunk, chunk + read);
    }

    data = std::move(buffer);
    LOG_DEBUG("Extracted compressed texture using stream, size: " + std::to_string(data.size()));
  }
  else {
    // 处理未压缩的RGBA纹理数据
    unsigned int pixelCount = aiTexture->mWidth * aiTexture->mHeight;
    unsigned int dataSize = pixelCount * 4;  // RGBA
    data.resize(dataSize);

    const aiTexel *texelData = aiTexture->pcData;

    // 将ARGB数据转换为RGBA
    for (unsigned int i = 0; i < pixelCount; ++i) {
      data[i * 4 + 0] = texelData[i].r;  // R
      data[i * 4 + 1] = texelData[i].g;  // G
      data[i * 4 + 2] = texelData[i].b;  // B
      data[i * 4 + 3] = texelData[i].a;  // A
    }

    LOG_DEBUG("Extracted uncompressed embedded texture, " + std::to_string(aiTexture->mWidth) +
              "x" + std::to_string(aiTexture->mHeight));
  }
  return data;
}
TextureAssetID TextureLoader::LoadEnvironmentTexture(TextureCache &cache,
                                                     const std::string &hdrPath,
                                                     bool flipVertical)
{
  if (hdrPath.empty()) {
    LOG_ERROR("HDR texture path is empty");
    return TextureAssetID();
  }

  // 检查缓存
  TextureAssetID existingId = FindTextureByPath(cache, hdrPath);
  if (existingId.IsValid()) {
    LOG_INFO("Environment texture already cached: " + hdrPath);
    return existingId;
  }

  return LoadEnvironmentTextureInternal(cache, hdrPath, flipVertical);
}
bool TextureLoader::IsEmbeddedTexturePath(const std::string &path)
{
  return !path.empty() && path[0] == '*';
}

TextureFormat TextureLoader::DetermineLDRTextureFormat(int channels)
{
  // LDR纹理一般是8位
  switch (channels) {
    case 1:
      return TextureFormat::R8;
    case 2:
      return TextureFormat::RG8;
    case 3:
      return TextureFormat::RGB8;
    case 4:
      return TextureFormat::RGBA8;
    default:
      LOG_WARN("Unsupported channel count: " + std::to_string(channels) + ", using RGBA8");
      return TextureFormat::RGBA8;
  }
}

void TextureLoader::SetupDefaultSamplingParams(TextureMetadata &metadata)
{
  // 设置默认的采样参数（这些可以在材质系统中被覆盖）
  metadata.wrapModeS = TextureWrapMode::Repeat;
  metadata.wrapModeT = TextureWrapMode::Repeat;
  metadata.minFilter = TextureFilterMode::LinearMipmapLinear;
  metadata.magFilter = TextureFilterMode::Linear;
  metadata.generateMipmaps = true;

  // 根据纹理类型调整默认参数
  if (metadata.sourcePath.find("normal") != std::string::npos) {
    // 法线贴图通常使用不同的过滤模式
    metadata.minFilter = TextureFilterMode::Linear;
    metadata.magFilter = TextureFilterMode::Linear;
  }
}

TextureAssetID TextureLoader::FindTextureByPath(TextureCache &cache, const std::string &path)
{
  // 基于路径生成ID进行查找
  TextureAssetID searchId(UUIDGenerator::Generate(path.c_str()));
  auto texture = cache.Get(searchId);

  if (texture) {
    // 验证路径是否匹配（防止哈希冲突）
    if (texture->metadata.sourcePath == path) {
      return searchId;
    }
  }

  return TextureAssetID();  // 返回无效ID表示未找到
}

TextureFormat TextureLoader::DetermineHDRTextureFormat(int channels)
{
  // HDR纹理默认32位
  switch (channels) {
    case 1:
      return TextureFormat::R32F;
    case 2:
      return TextureFormat::RG32F;
    case 3:
      return TextureFormat::RGB32F;
    case 4:
      return TextureFormat::RGBA32F;
    default:
      LOG_WARN("Unsupported HDR channel count: " + std::to_string(channels) + ", using RGB32F");
      return TextureFormat::RGB32F;
  }
}

void TextureLoader::SetupHDRSamplingParams(TextureMetadata &metadata)
{
  metadata.wrapModeS = TextureWrapMode::ClampToEdge;
  metadata.wrapModeT = TextureWrapMode::ClampToEdge;
  metadata.minFilter = TextureFilterMode::LinearMipmapLinear;
  metadata.magFilter = TextureFilterMode::Linear;
  metadata.generateMipmaps = true;  // HDR环境贴图通常需要mipmaps
}

size_t TextureLoader::GetPixelSize(TextureFormat format)
{
  switch (format) {
    // 8位格式
    case TextureFormat::R8:
      return 1;  // 1字节
    case TextureFormat::RG8:
      return 2;  // 2字节
    case TextureFormat::RGB8:
    case TextureFormat::SRGB8:
      return 3;  // 3字节
    case TextureFormat::RGBA8:
    case TextureFormat::SRGB8_ALPHA8:
      return 4;  // 4字节

    // 16位浮点格式
    case TextureFormat::R16F:
      return 2;  // 3通道 × 2字节 = 6字节
    case TextureFormat::RG16F:
      return 4;  // 3通道 × 2字节 = 6字节
    case TextureFormat::RGB16F:
      return 6;  // 3通道 × 2字节 = 6字节
    case TextureFormat::RGBA16F:
      return 8;  // 4通道 × 2字节 = 8字节

    // 32位浮点格式
    case TextureFormat::R32F:
      return 4;  // 1通道 × 4字节 = 4字节
    case TextureFormat::RG32F:
      return 8;  // 2通道 × 4字节 = 8字节
    case TextureFormat::RGB32F:
      return 12;  // 3通道 × 4字节 = 12字节
    case TextureFormat::RGBA32F:
      return 16;  // 4通道 × 4字节 = 16字节

    // 深度/模板格式
    case TextureFormat::STENCIL_INDEX8:
      return 1;  //  8位 = 1字节
    case TextureFormat::DEPTH_COMPONENT16:
    case TextureFormat::STENCIL_INDEX16:
      return 2;  // 16位 = 2字节
    case TextureFormat::DEPTH_COMPONENT24:
      return 3;  // 24位 = 3字节
    case TextureFormat::DEPTH_COMPONENT32:
      return 4;  // 32位 = 4字节
    case TextureFormat::DEPTH24_STENCIL8:
      return 4;  // 24 + 8位 = 4字节

    default:
      LOG_WARN("Unknown texture format, assuming 4 bytes per pixel");
      return 4;
  }
}
bool TextureLoader::GenerateAndLoadIBLTextures(TextureCache &cache,
                                               const std::string &hdrPath,
                                               const std::string &environmentId)
{
  std::string cachePath = GetIBLCachePath(environmentId);

  // 检查是否需要重新生成
  if (!ShouldGenerateIBL(environmentId)) {
    LOG_INFO("Using cached IBL textures for: " + environmentId);
    return true;
  }

  // 配置生成选项
  IBLGenerator::GenerateOptions options = IBLGenerator::GetOptionsForQuality(
      s_IBLConfig.qualityLevel);

  LOG_INFO("Generating IBL textures for environment: " + environmentId);

  // 生成IBL纹理
  if (!IBLGenerator::GenerateIBLTextures(hdrPath, cachePath, options)) {
    LOG_ERROR("Failed to generate IBL textures for: " + environmentId);
    return false;
  }

  // 加载生成的纹理
  std::string environmentMapPath = cachePath + "/environment.ktx2";
  std::string brdfLUTPath = cachePath + "/brdf_lut.ktx2";

  TextureAssetID envMapId = LoadTexture(cache, environmentMapPath, 0, false);
  TextureAssetID brdfLUTId = LoadTexture(cache, brdfLUTPath, 0, false);

  if (!envMapId.IsValid() || !brdfLUTId.IsValid()) {
    LOG_ERROR("Failed to load generated IBL textures");
    return false;
  }

  LOG_INFO("Successfully generated and loaded IBL textures for: " + environmentId);
  return true;
}
std::string TextureLoader::GetIBLCachePath(const std::string &environmentId)
{
  return s_IBLConfig.cacheDirectory + "/" + environmentId;
}
bool TextureLoader::ShouldGenerateIBL(const std::string &environmentId)
{
  std::string cachePath = GetIBLCachePath(environmentId);
  std::string environmentMapPath = cachePath + "/environment.ktx2";
  std::string brdfLUTPath = cachePath + "/brdf_lut.ktx2";

  // 如果缓存文件不存在，需要生成
  if (!std::filesystem::exists(environmentMapPath) || !std::filesystem::exists(brdfLUTPath)) {
    return true;
  }

  return false;
}

};  // namespace mite