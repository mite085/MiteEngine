#include "texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace mite {
std::pair<TextureMetadata, std::unique_ptr<uint8_t[], void (*)(uint8_t *)>> TextureLoader::
    LoadTextureData(
    const std::string &path, int desiredChannels, bool flipVertical)
{
  // 初始化stb_image配置
  stbi_set_flip_vertically_on_load(flipVertical);  // OpenGL坐标系需要翻转Y轴

  TextureMetadata metadata;
  metadata.path = path;
  metadata.isHDR = false;  // 明确标记为非HDR

  // 加载图像数据（自动根据desiredChannels转换格式）
  uint8_t *pixelData = stbi_load(
      path.c_str(), &metadata.width, &metadata.height, &metadata.channels, desiredChannels);

  // 检查加载结果
  if (!pixelData) {
    LOG_ERROR("Failed to load texture: " + path + ", reason: " + stbi_failure_reason());
  }

  // 更新实际通道数（如果desiredChannels为0则保留原通道）
  if (desiredChannels > 0) {
    metadata.channels = desiredChannels;
  }

  // 使用函数指针类型的删除器
  auto deleter = [](uint8_t *data) { stbi_image_free(data); };
  std::unique_ptr<uint8_t[], decltype(deleter)> managedData(pixelData, deleter);

  return {metadata, std::move(managedData)};
}

void TextureLoader::FreeTextureData(void *data)
{
  stbi_image_free(data);
}
};
