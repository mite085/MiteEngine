#include "texture_loader.h"
#include "basic_event/asset_event.h"
#include <assimp/scene.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace mite {
std::shared_ptr<TextureAsset> TextureLoader::LoadTexture(const std::string &path,
                                                         int desiredChannels,
                                                         bool flipVertical)
{

  // 检查路径有效性
  if (path.empty()) {
    LOG_ERROR("Texture path is empty");
    return nullptr;
  }
  // 嵌入式纹理需要特殊处理
  if (IsEmbeddedTexturePath(path)) {
    LOG_ERROR("External texture path should not be embedded format: " + path);
    return nullptr;
  }
  // 调用内部加载实现，嵌入式数据为空
  return LoadTextureInternal(path, {}, desiredChannels, flipVertical);
}

std::shared_ptr<TextureAsset> TextureLoader::LoadEmbeddedTexture(const std::string &embeddedId,
                                                                 const std::string &modelPath,
                                                                 const aiTexture *aiTexture,
                                                                 int desiredChannels,
                                                                 bool flipVertical)
{

  // 参数检查
  if (!aiTexture) {
    LOG_ERROR("Null aiTexture provided for embedded texture");
    return nullptr;
  }
  if (embeddedId.empty() || embeddedId[0] != '*') {
    LOG_ERROR("Invalid embedded texture ID: " + embeddedId);
    return nullptr;
  }
  // 生成唯一的纹理路径标识：模型路径 + 嵌入式ID
  std::string texturePath = modelPath + embeddedId;
  // 从Assimp纹理提取嵌入式数据
  std::vector<uint8_t> embeddedData = ExtractEmbeddedData(aiTexture);
  if (embeddedData.empty()) {
    LOG_ERROR("Failed to extract embedded data from aiTexture: " + texturePath);
    return nullptr;
  }
  // 调用内部加载实现
  return LoadTextureInternal(texturePath, embeddedData, desiredChannels, flipVertical);
}

std::shared_ptr<TextureAsset> TextureLoader::LoadTextureInternal(
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
    return nullptr;
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
  textureAsset->metadata.channelCount = static_cast<uint32_t>(actualChannels);
  textureAsset->metadata.sourceFormat = DetermineTextureFormat(actualChannels);
  textureAsset->metadata.target = DetermineTextureTarget(path);
  textureAsset->metadata.isSRGB = true;  // 默认假设为sRGB纹理
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
  LOG_INFO("Successfully loaded texture: " + path + " (" + std::to_string(width) + "x" +
           std::to_string(height) + ", channels: " + std::to_string(actualChannels) + ")");
  return textureAsset;
}

std::vector<uint8_t> TextureLoader::ExtractEmbeddedData(const aiTexture *aiTexture)
{
  std::vector<uint8_t> data;
  if (!aiTexture) {
    return data;
  }
  // 处理压缩格式纹理（JPEG/PNG等）
  if (aiTexture->mHeight == 0) {
    data.assign(aiTexture->pcData, aiTexture->pcData + aiTexture->mWidth);
  }
  else {
    // 返回空数据，表示不支持“未压缩ARGB数据”（很少见，需要格式转换）
    LOG_WARN("Uncompressed embedded texture detected, format conversion may be needed");
  }
  return data;
}

bool TextureLoader::IsEmbeddedTexturePath(const std::string &path)
{
  return !path.empty() && path[0] == '*';
}

TextureFormat TextureLoader::DetermineTextureFormat(int channels)
{
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

TextureTarget TextureLoader::DetermineTextureTarget(const std::string &path)
{
  // 根据文件扩展名或路径特征推断纹理目标类型
  // 默认使用2D纹理，后续可根据需要扩展
  return TextureTarget::TEXTURE_2D;
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

};  // namespace mite