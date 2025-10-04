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

  // 直接创建帧缓冲，FrameBuffer会自动创建所有运行时纹理
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
  if (!m_framebuffer->IsComplete()) {
    LOG_ERROR("GBuffer validation failed: framebuffer is incomplete");
    return false;
  }
  // 验证所有纹理附件是否有效
  for (int i = 0; i < COUNT; ++i) {
    auto texture = getTexture(static_cast<GBufferIndex>(i));
    if (!texture || !texture->isValid()) {
      LOG_ERROR("GBuffer validation failed: texture at index {} is invalid", i);
      return false;
    }
    // 检查纹理尺寸是否匹配
    if (texture->getWidth() != m_width || texture->getHeight() != m_height) {
      LOG_ERROR("GBuffer validation failed: texture size mismatch at index {}", i);
      return false;
    }
  }
  LOG_DEBUG("GBuffer validation passed");
  return true;
}

bool GBuffer::resize(uint32_t newWidth, uint32_t newHeight)
{
  if (newWidth <= 0 || newHeight <= 0) {
    LOG_ERROR("Invalid resize dimensions: {}x{}", newWidth, newHeight);
    return false;
  }
  if (newWidth == m_width && newHeight == m_height) {
    return true;
  }
  LOG_INFO("Resizing GBuffer from {}x{} to {}x{}", m_width, m_height, newWidth, newHeight);
  // 直接调用FrameBuffer的Resize方法，自动重新创建所有纹理
  if (m_framebuffer) {
    m_framebuffer->Resize(newWidth, newHeight);
    m_width = newWidth;
    m_height = newHeight;

    if (!validate()) {
      LOG_ERROR("GBuffer validation failed after resize");
      return false;
    }

    return true;
  }
  return false;
}
RuntimeTexturePtr GBuffer::getTexture(GBufferIndex index) const
{
  if (!m_framebuffer || index < 0 || index >= COUNT) {
    LOG_ERROR("Invalid texture index or framebuffer: {}", static_cast<int>(index));
    return nullptr;
  }
  // 直接从FrameBuffer获取对应的颜色附件
  return m_framebuffer->GetColorAttachment(static_cast<uint32_t>(index));
}

std::shared_ptr<FrameBuffer> GBuffer::getFramebuffer() const
{
  return m_framebuffer;
}
int GBuffer::getWidth() const
{
  return m_width;
}
int GBuffer::getHeight() const
{
  return m_height;
}
bool GBuffer::isValid() const
{
  return m_isValid;
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

  // 为每个G-Buffer纹理创建附件规格
  for (int i = 0; i < COUNT; ++i) {
    FrameBufferAttachmentSpec attachment;
    attachment.type = RuntimeTexture::RuntimeTextureType::GBufferMap;
    attachment.internalFormat = getTextureFormat(static_cast<GBufferIndex>(i));
    attachment.generateMipmaps = false;  // G-Buffer不需要mipmap
    spec.attachments.push_back(attachment);
  }

  return spec;
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

}  // namespace mite