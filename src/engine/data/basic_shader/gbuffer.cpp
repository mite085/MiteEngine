#include "gbuffer.h"

#include "basic_shader/uniform_buffer.h"
namespace mite {
// 静态成员定义
const std::map<RuntimeTextureType, uint32_t> GBuffer::TextureTypeToIndex = {
    {RuntimeTextureType::GBuffer_WorldPosDepth, 0},
    {RuntimeTextureType::GBuffer_BaseColorMatType, 1},
    {RuntimeTextureType::GBuffer_MetallicRoughnessAO, 2},
    {RuntimeTextureType::GBuffer_NormalScale, 3},
    {RuntimeTextureType::GBuffer_EmissionAlpha, 4},
    {RuntimeTextureType::GBuffer_NPRParam, 5},
    {RuntimeTextureType::GBuffer_NPRColor, 6}};
const std::vector<RuntimeTextureType> &GBuffer::GetTextureTypes() {
  static const std::vector<RuntimeTextureType> types = {
      RuntimeTextureType::GBuffer_WorldPosDepth,
      RuntimeTextureType::GBuffer_BaseColorMatType,
      RuntimeTextureType::GBuffer_MetallicRoughnessAO,
      RuntimeTextureType::GBuffer_NormalScale,
      RuntimeTextureType::GBuffer_EmissionAlpha,
      RuntimeTextureType::GBuffer_NPRParam,
      RuntimeTextureType::GBuffer_NPRColor};
  return types;
}
const char *GBuffer::GetTextureTypeName(RuntimeTextureType type) {
  switch (type) {
    case RuntimeTextureType::GBuffer_WorldPosDepth:
      return ShaderBufferResourceNames::GBUFFER_WORLD_POS_DEPTH;
    case RuntimeTextureType::GBuffer_BaseColorMatType:
      return ShaderBufferResourceNames::GBUFFER_BASE_COLOR_MAT_TYPE;
    case RuntimeTextureType::GBuffer_MetallicRoughnessAO:
      return ShaderBufferResourceNames::GBUFFER_METALLIC_ROUGHNESS_AO;
    case RuntimeTextureType::GBuffer_NormalScale:
      return ShaderBufferResourceNames::GBUFFER_NORMAL_SCALE;
    case RuntimeTextureType::GBuffer_EmissionAlpha:
      return ShaderBufferResourceNames::GBUFFER_EMISSION_ALPHA;
    case RuntimeTextureType::GBuffer_NPRParam:
      return ShaderBufferResourceNames::GBUFFER_NPR_PARAM;
    case RuntimeTextureType::GBuffer_NPRColor:
      return ShaderBufferResourceNames::GBUFFER_NPR_COLOR;
    default:
      return "Unknown_GBuffer_Type";
  }
}
GBuffer::GBuffer() { LOG_TRACE("GBuffer constructor called"); }

GBuffer::~GBuffer() {
  cleanup();
  LOG_TRACE("GBuffer destructor called");
}

bool GBuffer::create() {
  // 如果已经初始化，先清理
  if (m_isValid) {
    LOG_WARN("GBuffer already initialized, cleaning up first");
    cleanup();
  }

  LOG_INFO("Initializing GBuffer with size {}x{}", m_width, m_height);

  // 直接创建帧缓冲，FrameBuffer会自动创建所有运行时纹理
  FrameBufferSpec spec = CreateFrameBufferSpec();
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
  LOG_INFO("GBuffer initialized successfully with {} textures", TEXTURE_COUNT);

  return true;
}

void GBuffer::cleanup() {
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

bool GBuffer::validate() const {
  if (!m_framebuffer) {
    LOG_ERROR("GBuffer validation failed: framebuffer is null");
    return false;
  }
  if (!m_framebuffer->IsComplete()) {
    LOG_ERROR("GBuffer validation failed: framebuffer is incomplete");
    return false;
  }
  // 验证所有纹理附件是否有效
  for (const auto &type : GetTextureTypes()) {
    auto texture = GetTexture(type);
    if (!texture || !texture->IsValid()) {
      LOG_ERROR("GBuffer validation failed: texture is invalid");
      return false;
    }
    // 检查纹理尺寸是否匹配
    if (texture->GetWidth() != m_width || texture->GetHeight() != m_height) {
      LOG_ERROR("GBuffer validation failed: texture size mismatch");
      return false;
    }
  }
  LOG_DEBUG("GBuffer validation passed");
  return true;
}

bool GBuffer::resize(uint32_t newWidth, uint32_t newHeight) {
  if (newWidth <= 0 || newHeight <= 0) {
    LOG_ERROR("Invalid resize dimensions: {}x{}", newWidth, newHeight);
    return false;
  }
  if (newWidth == m_width && newHeight == m_height) {
    return true;
  }
  LOG_INFO("Resizing GBuffer from {}x{} to {}x{}", m_width, m_height, newWidth,
           newHeight);
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
RuntimeTexturePtr GBuffer::GetTexture(RuntimeTextureType type) const {
  if (!m_framebuffer) {
    LOG_ERROR("Cannot get texture: framebuffer is null");
    return nullptr;
  }
  auto it = TextureTypeToIndex.find(type);
  if (it == TextureTypeToIndex.end()) {
    LOG_ERROR("Cannot get texture: type out of range");
    return nullptr;
  }

  // 直接从FrameBuffer获取对应的颜色附件
  return m_framebuffer->GetColorAttachment(static_cast<uint32_t>(it->second));
}

std::shared_ptr<FrameBuffer> GBuffer::GetFramebuffer() const {
  return m_framebuffer;
}
int GBuffer::GetWidth() const { return m_width; }
int GBuffer::GetHeight() const { return m_height; }
bool GBuffer::IsValid() const { return m_isValid; }

void GBuffer::bind() const {
  if (m_framebuffer && m_isValid) {
    m_framebuffer->Bind();
  } else {
    LOG_WARN("Attempted to bind invalid GBuffer");
  }
}

void GBuffer::unbind() const {
  if (m_framebuffer) {
    m_framebuffer->Unbind();
  }
}

FrameBufferSpec GBuffer::CreateFrameBufferSpec() const {
  FrameBufferSpec spec;
  spec.width = m_width;
  spec.height = m_height;
  spec.samples = 1;  // G-Buffer通常不使用多重采样

  // 为每个G-Buffer纹理创建附件规格
  for (const auto &type : GetTextureTypes()) {
    FrameBufferAttachmentSpec attachment;
    attachment.type = type;
    attachment.internalFormat = GetTextureFormat(type);
    attachment.generateMipmaps = false;  // G-Buffer不需要mipmap
    spec.attachments.push_back(attachment);
  }

  // 创建深度附件
  FrameBufferAttachmentSpec depthAttachment;
  depthAttachment.type = RuntimeTextureType::Depth;
  depthAttachment.internalFormat = TextureFormat::DEPTH_COMPONENT16;
  depthAttachment.generateMipmaps = false;
  spec.attachments.push_back(depthAttachment);

  return spec;
}

TextureFormat GBuffer::GetTextureFormat(RuntimeTextureType index) const {
  switch (index) {
    case RuntimeTextureType::GBuffer_WorldPosDepth:
      return TextureFormat::RGBA32F;  // 世界坐标需要高精度

    case RuntimeTextureType::GBuffer_BaseColorMatType:
    case RuntimeTextureType::GBuffer_MetallicRoughnessAO:
    case RuntimeTextureType::GBuffer_NormalScale:
    case RuntimeTextureType::GBuffer_EmissionAlpha:
    case RuntimeTextureType::GBuffer_NPRParam:
    case RuntimeTextureType::GBuffer_NPRColor:
    case RuntimeTextureType::Depth:
      return TextureFormat::RGBA16F;  // 其他普通精度即可

    default:
      LOG_WARN("Unknown texture index: {}, using RGBA16F as default",
               static_cast<int>(index));
      return TextureFormat::RGBA16F;
  }
}
}  // namespace mite