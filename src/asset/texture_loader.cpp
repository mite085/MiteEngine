#include "texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace mite {
std::shared_ptr<TextureAsset> TextureLoader::LoadTextureData(const std::string &path,
                                                             int desiredChannels,
                                                             bool flipVertical)
{
  // 1. 初始化stb_image配置
  stbi_set_flip_vertically_on_load(flipVertical);  // OpenGL坐标系需要翻转Y轴

  TextureMetadata metadata;
  metadata.path = path;
  metadata.isHDR = false;  // 明确标记为非HDR

  // 2. 加载图像数据（自动根据desiredChannels转换格式）
  uint8_t *pixelData = stbi_load(
      path.c_str(), &metadata.width, &metadata.height, &metadata.channels, desiredChannels);

  // 3. 检查加载结果
  if (!pixelData) {
    LOG_ERROR("Failed to load texture: " + path + ", reason: " + stbi_failure_reason());
  }

  // 4. 更新实际通道数（如果desiredChannels为0则保留原通道）
  if (desiredChannels > 0) {
    metadata.channels = desiredChannels;
  }

  // 5. 使用函数指针类型的删除器
  auto deleter = [](uint8_t *data) { stbi_image_free(data); };
  std::unique_ptr<uint8_t[], decltype(deleter)> managedData(pixelData, deleter);

  // 6. 构建资产
  std::shared_ptr<TextureAsset> textureAsset;
  textureAsset->id = UUIDGenerator::Generate(path.c_str());
  textureAsset->metadata = metadata;
  textureAsset->textureData.textureData = std::move(managedData);

  // 7. (该步骤移交给RendererDevice处理)
  //    转换为Renderer模块的TextureSourceData
  //TextureSourceData rendererData;
  //rendererData.pixelData = pixelData;
  //rendererData.width = metadata.width;
  //rendererData.height = metadata.height;
  //rendererData.format = metadata.format;
  //rendererData.wrapMode = TextureWrapMode::Repeat;  // 默认值或从配置读取
  //rendererData.filterMode = TextureFilterMode::Linear;
  //rendererData.generateMipmaps = true;

  // 8. 发布事件，委托RendererDevice创建GPU资源
  TextureLoadEvent event(textureAsset);
  EventBus::Get().Post(event);
  //textureAsset->handle = IRenderDevice::Current().CreateTexture(rendererData);

  return textureAsset;
}

void TextureLoader::FreeTextureData(void *data)
{
  stbi_image_free(data);
}
};  // namespace mite