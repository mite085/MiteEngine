#include "runtime_texture.h"
#include "basic_event/asset_event.h"
#include "subscription_group.h"

namespace mite {
RuntimeTexture::~RuntimeTexture()
{
  cleanup();
}

bool RuntimeTexture::initialize(RuntimeTextureType type,
                                int width,
                                int height,
                                TextureFormat format)
{
  // 参数验证
  if (width <= 0 || height <= 0) {
    LOG_ERROR("Invalid texture dimensions: {}x{}", width, height);
    return false;
  }

  // 如果已经初始化，先清理
  if (isValid()) {
    LOG_WARN("RuntimeTexture already initialized, cleaning up first");
    cleanup();
  }

  // 存储属性
  m_type = type;
  m_width = width;
  m_height = height;
  m_format = format;

  // 创建纹理创建信息
  std::shared_ptr<TextureCreateInfo> createInfo = std::make_shared<TextureCreateInfo>();
  createInfo->width = width;
  createInfo->height = height;
  createInfo->format = format;
  createInfo->generateMipmaps = false;  // G-Buffer通常不需要mipmap

  // 根据纹理类型设置特定参数
  switch (type) {
    case RuntimeTextureType::ShadowMap_Directional:
    case RuntimeTextureType::ShadowMap_Point:
    case RuntimeTextureType::ShadowMap_Spot:
    case RuntimeTextureType::ShadowMap_Area:
      // 阴影贴图：最近邻过滤、延申边框颜色
      createInfo->minFilter = TextureFilterMode::Nearest;
      createInfo->magFilter = TextureFilterMode::Nearest;
      createInfo->wrapModeS = TextureWrapMode::ClampToBorder;
      createInfo->wrapModeT = TextureWrapMode::ClampToBorder;
      break;

    case RuntimeTextureType::Stencil:
    case RuntimeTextureType::Depth:
    case RuntimeTextureType::GBuffer_WorldPosDepth:
    case RuntimeTextureType::GBuffer_BaseColorMatType:
    case RuntimeTextureType::GBuffer_MetallicRoughnessAO:
    case RuntimeTextureType::GBuffer_NormalScale:
    case RuntimeTextureType::GBuffer_EmissionAlpha:
    case RuntimeTextureType::GBuffer_NPRParam:
    case RuntimeTextureType::GBuffer_NPRColor:
      // G-Buffer纹理/深度模板缓冲：最近邻过滤、边缘拉伸
      // 注意，G-Buffer不能使用线性过滤，否则会引入采样误差
      createInfo->minFilter = TextureFilterMode::Nearest;
      createInfo->magFilter = TextureFilterMode::Nearest;
      createInfo->wrapModeS = TextureWrapMode::ClampToEdge;
      createInfo->wrapModeT = TextureWrapMode::ClampToEdge;
      break;

    case RuntimeTextureType::Lighting_Diffuse:
    case RuntimeTextureType::Lighting_Specular:
    case RuntimeTextureType::Lighting_Combined:
    case RuntimeTextureType::Lighting_Ambient:
    case RuntimeTextureType::RenderTarget:
    default:
      // 光照着色结果/普通渲染目标/默认情况：线性过滤、边缘拉伸（防止接缝）
      createInfo->minFilter = TextureFilterMode::Linear;
      createInfo->magFilter = TextureFilterMode::Linear;
      createInfo->wrapModeS = TextureWrapMode::ClampToEdge;
      createInfo->wrapModeT = TextureWrapMode::ClampToEdge;
      break;
  }

  // 发布事件委托OpenGLDevice创建纹理
  std::function<void(TextureGPUHandle)> onComplete = [this](TextureGPUHandle handle) {
    m_handle = handle;
  };
  EventBus::Publish<RuntimeTextureCreateEvent>(RuntimeTextureCreateEvent(createInfo, onComplete));

  if (!isValid()) {
    LOG_ERROR("Failed to create runtime texture of type {} ({}x{})",
              static_cast<int>(type),
              width,
              height);
    return false;
  }

  LOG_DEBUG("Created runtime texture: type={}, size={}x{}, format={}",
            static_cast<int>(type),
            width,
            height,
            static_cast<int>(format));

  return true;
}

void RuntimeTexture::cleanup()
{
  if (isValid()) {
    // 发布事件委托OpenGLDevice销毁纹理
    EventBus::Publish<RuntimeTextureDestroyRequestEvent>(
        RuntimeTextureDestroyRequestEvent(m_handle));
    m_handle.apiHandle = 0;

    LOG_DEBUG("Cleaned up runtime texture: type={}", static_cast<int>(m_type));
  }

  // 重置状态
  m_width = 0;
  m_height = 0;
  m_format = TextureFormat::RGBA8;
}

bool RuntimeTexture::resize(int newWidth, int newHeight)
{
  if (newWidth <= 0 || newHeight <= 0) {
    LOG_ERROR("Invalid resize dimensions: {}x{}", newWidth, newHeight);
    return false;
  }

  if (newWidth == m_width && newHeight == m_height) {
    // 尺寸未变化，无需调整
    return true;
  }

  LOG_INFO("Resizing runtime texture from {}x{} to {}x{}", m_width, m_height, newWidth, newHeight);

  // 保存原有类型和格式
  RuntimeTextureType oldType = m_type;
  TextureFormat oldFormat = m_format;

  // 清理旧资源
  cleanup();

  // 使用新尺寸重新初始化
  return initialize(oldType, newWidth, newHeight, oldFormat);
}
}  // namespace mite