#include "texture_loader.h"

#include <assimp/MemoryIOWrapper.h>
#include <assimp/scene.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace mite {
TextureAssetID TextureLoader::LoadTexture(TextureCache &cache,
                                          const std::string &path,
                                          int desiredChannels,
                                          bool flipVertical) {
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

  // 调用内部加载实现，嵌入式数据为空
  return LoadTextureInternal(cache, path, {}, desiredChannels, flipVertical);
}

TextureAssetID TextureLoader::LoadEmbeddedTexture(TextureCache &cache,
                                                  const std::string &embeddedId,
                                                  const std::string &modelPath,
                                                  const aiTexture *aiTexture,
                                                  int desiredChannels,
                                                  bool flipVertical) {
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
  return LoadTextureInternal(cache, texturePath, embeddedData, desiredChannels,
                             flipVertical);
}

TextureAssetID TextureLoader::LoadTextureInternal(
    TextureCache &cache, const std::string &path,
    const std::vector<uint8_t> &embeddedData, int desiredChannels,
    bool flipVertical) {
  // 设置STB图像加载配置
  stbi_set_flip_vertically_on_load(flipVertical);

  // 准备加载变量
  int width = 0, height = 0, channels = 0;
  uint8_t *pixelData = nullptr;

  // 区分外部文件和嵌入式数据加载
  if (embeddedData.empty()) {
    // 外部文件加载
    pixelData =
        stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
  } else {
    // 嵌入式数据加载
    pixelData = stbi_load_from_memory(
        embeddedData.data(), static_cast<int>(embeddedData.size()), &width,
        &height, &channels, desiredChannels);
  }
  // 检查加载结果
  if (!pixelData) {
    LOG_ERROR("Failed to load texture: " + path +
              ", reason: " + stbi_failure_reason());
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
  textureAsset->metadata.sourceFormat = DetermineTextureFormat(actualChannels);
  textureAsset->metadata.target = DetermineTextureTarget(path);

  // 设置默认采样参数
  SetupDefaultSamplingParams(textureAsset->metadata);

  // 创建像素数据向量并转移所有权
  size_t dataSize = width * height * actualChannels;
  std::vector<uint8_t> pixelVector(pixelData, pixelData + dataSize);

  // 释放STB分配的内存
  stbi_image_free(pixelData);

  // 生成TextureSourceData供Renderer使用
  TextureSourceData sourceData =
      textureAsset->metadata.generateSourceData(std::move(pixelVector));

  // 发布纹理加载事件，委托Renderer创建GPU资源
  EventBus::Publish<TextureLoadEvent>(
      std::make_shared<TextureSourceData>(std::move(sourceData)), textureAsset);

  // 将纹理资产存入缓存
  if (cache.Store(textureAsset)) {
    TextureAssetID textureId = textureAsset->GetID();
    LOG_INFO("Successfully loaded and cached texture: " + path + " (" +
             std::to_string(width) + "x" + std::to_string(height) +
             ", channels: " + std::to_string(actualChannels) + ")");
    return textureId;
  } else {
    LOG_ERROR("Failed to store texture in cache: " + path);
    return TextureAssetID();
  }
}

std::vector<uint8_t> TextureLoader::ExtractEmbeddedData(
    const aiTexture *aiTexture) {
  std::vector<uint8_t> data;
  if (!aiTexture) return data;
  if (aiTexture->mHeight == 0) {
    // 使用Assimp的内存IO接口
    Assimp::MemoryIOStream stream(
        reinterpret_cast<const uint8_t *>(aiTexture->pcData),
        aiTexture->mWidth);

    // 读取直到EOF
    std::vector<uint8_t> buffer;
    const size_t chunkSize = 4096;
    uint8_t chunk[chunkSize];

    while (true) {
      size_t read = stream.Read(chunk, 1, chunkSize);
      if (read == 0) break;
      buffer.insert(buffer.end(), chunk, chunk + read);
    }

    data = std::move(buffer);
    LOG_DEBUG("Extracted compressed texture using stream, size: " +
              std::to_string(data.size()));
  } else {
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

    LOG_DEBUG("Extracted uncompressed embedded texture, " +
              std::to_string(aiTexture->mWidth) + "x" +
              std::to_string(aiTexture->mHeight));
  }
  return data;
}

bool TextureLoader::IsEmbeddedTexturePath(const std::string &path) {
  return !path.empty() && path[0] == '*';
}

TextureFormat TextureLoader::DetermineTextureFormat(int channels) {
  // 目前仅支持8位无符号归一化格式，
  // 若要支持其他格式，需要先扩展uint8_t *pixelData

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
      LOG_WARN("Unsupported channel count: " + std::to_string(channels) +
               ", using RGBA8");
      return TextureFormat::RGBA8;
  }
}

TextureTarget TextureLoader::DetermineTextureTarget(
    [[maybe_unused]] const std::string &path) {
  // TODO：根据文件扩展名或路径特征推断纹理目标类型
  // 默认使用2D纹理，后续可根据需要扩展
  return TextureTarget::TEXTURE_2D;
}

void TextureLoader::SetupDefaultSamplingParams(TextureMetadata &metadata) {
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

TextureAssetID TextureLoader::FindTextureByPath(TextureCache &cache,
                                                const std::string &path) {
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
};  // namespace mite