#include "gbuffer.h"

namespace mite {
GBuffer::GBuffer()
{
  LOG_TRACE("GBuffer constructor called");
}

GBuffer::~GBuffer()
{
  cleanup();
  LOG_TRACE("GBuffer destructor called");
}

bool GBuffer::create(uint32_t width, uint32_t height)
{
  // 参数验证
  if (width <= 0 || height <= 0) {
    LOG_ERROR("Invalid GBuffer dimensions: {}x{}", width, height);
    return false;
  }

  // 如果已经初始化，先清理
  if (m_isValid) {
    LOG_WARN("GBuffer already initialized, cleaning up first");
    cleanup();
  }

  // 存储尺寸
  m_width = width;
  m_height = height;

  LOG_INFO("Initializing GBuffer with size {}x{}", width, height);

  // 创建所有G-Buffer纹理
  for (int i = 0; i < COUNT; ++i) {
    GBufferIndex index = static_cast<GBufferIndex>(i);
    if (!createTexture(index)) {
      LOG_ERROR("Failed to create GBuffer texture at index {}", i);
      cleanup();
      return false;
    }
  }

  // 创建帧缓冲（在纹理创建之后执行）
  FrameBufferSpec spec = createFrameBufferSpec();
  m_framebuffer = std::make_shared<FrameBuffer>(spec);

  if (!m_framebuffer->IsComplete()) {
    LOG_ERROR("GBuffer framebuffer is incomplete");
    cleanup();
    return false;
  }

  // 验证G-Buffer完整性
  if (!validate()) {
    LOG_ERROR("GBuffer validation failed");
    cleanup();
    return false;
  }

  m_isValid = true;
  LOG_INFO("GBuffer initialized successfully with {} textures", static_cast<int>(COUNT));

  return true;
}

void GBuffer::cleanup()
{
  // 清理帧缓冲
  if (m_framebuffer) {
    m_framebuffer.reset();
  }

  // 清理所有纹理
  for (auto &texture : m_textures) {
    if (texture) {
      texture->cleanup();
      texture.reset();
    }
  }

  // 重置状态
  m_width = 0;
  m_height = 0;
  m_isValid = false;

  LOG_DEBUG("GBuffer resources cleaned up");
}

bool GBuffer::validate() const
{
  if (!m_framebuffer) {
    LOG_ERROR("GBuffer validation failed: framebuffer is null");
    return false;
  }

  // 检查帧缓冲完整性
  if (!m_framebuffer->IsComplete()) {
    LOG_ERROR("GBuffer validation failed: framebuffer is incomplete");
    return false;
  }

  // 检查所有纹理是否有效
  for (int i = 0; i < COUNT; ++i) {
    if (!m_textures[i] || !m_textures[i]->isValid()) {
      LOG_ERROR("GBuffer validation failed: texture at index {} is invalid", i);
      return false;
    }

    // 检查纹理尺寸是否匹配
    if (m_textures[i]->getWidth() != m_width || m_textures[i]->getHeight() != m_height) {
      LOG_ERROR("GBuffer validation failed: texture size mismatch at index {}", i);
      return false;
    }
  }

  LOG_DEBUG("GBuffer validation passed");
  return true;
}

RuntimeTexture *GBuffer::getTexture(GBufferIndex index) const
{
  if (index < 0 || index >= COUNT) {
    LOG_ERROR("Invalid texture index: {}", static_cast<int>(index));
    return nullptr;
  }

  if (!m_textures[index]) {
    LOG_ERROR("Texture at index {} is null", static_cast<int>(index));
    return nullptr;
  }

  return m_textures[index].get();
}

void GBuffer::bind() const
{
  if (m_framebuffer && m_isValid) {
    m_framebuffer->Bind();
  }
  else {
    LOG_WARN("Attempted to bind invalid GBuffer");
  }
}

void GBuffer::unbind() const
{
  if (m_framebuffer) {
    m_framebuffer->Unbind();
  }
}

FrameBufferSpec GBuffer::createFrameBufferSpec() const
{
  FrameBufferSpec spec;
  spec.width = m_width;
  spec.height = m_height;
  spec.samples = 1;  // G-Buffer通常不使用多重采样

  // 添加颜色附件规格

  for (auto &texture : m_textures) {
    FrameBufferAttachmentSpec attachment;
    attachment.type = FrameBufferAttachmentType::Color;
    attachment.internalFormat = texture->getFormat();  // 根据RuntimeTexture确定Format
    attachment.generateMipmaps = false;                // G-Buffer不需要mipmap
    spec.attachments.push_back(attachment);
  }

  return spec;
}

bool GBuffer::createTexture(GBufferIndex index)
{
  if (index < 0 || index >= COUNT) {
    LOG_ERROR("Invalid texture index for creation: {}", static_cast<int>(index));
    return false;
  }

  // 创建运行时纹理
  auto texture = std::make_unique<RuntimeTexture>();
  RuntimeTexture::RuntimeTextureType type = getRuntimeTextureType(index);
  TextureFormat format = getTextureFormat(index);

  if (!texture->initialize(type, m_width, m_height, format)) {
    LOG_ERROR("Failed to initialize GBuffer texture at index {}", static_cast<int>(index));
    return false;
  }

  m_textures[index] = std::move(texture);
  LOG_DEBUG("Created GBuffer texture at index {}: type={}, format={}",
            static_cast<int>(index),
            static_cast<int>(type),
            static_cast<int>(format));

  return true;
}

TextureFormat GBuffer::getTextureFormat(GBufferIndex index) const
{
  switch (index) {
    case GBUFFER_WORLDPOS_DEPTH:
      return TextureFormat::RGBA16F;
    case GBUFFER_BASECOLOR_MATTYPE:
    case GBUFFER_METALLICROUGHNESS_AO:
    case GBUFFER_NORMAL_SCALE:
    case GBUFFEE_EMISSION_ALPHA:
    case GBUFFER_NPR_PARAM:
      return TextureFormat::RGBA16F;
    default:
      LOG_WARN("Unknown texture index: {}, using GBuffer0 as default", static_cast<int>(index));
      return TextureFormat::RGBA16F;
  }
}

RuntimeTexture::RuntimeTextureType GBuffer::getRuntimeTextureType(GBufferIndex index) const
{
  switch (index) {
    case GBUFFER_WORLDPOS_DEPTH:
    case GBUFFER_BASECOLOR_MATTYPE:
    case GBUFFER_METALLICROUGHNESS_AO:
    case GBUFFER_NORMAL_SCALE:
    case GBUFFEE_EMISSION_ALPHA:
    case GBUFFER_NPR_PARAM:
      return RuntimeTexture::RuntimeTextureType::GBufferMap;
    default:
      LOG_WARN("Unknown texture index: {}, using GBuffer as default", static_cast<int>(index));
      return RuntimeTexture::RuntimeTextureType::GBufferMap;
  }
}
}  // namespace mite