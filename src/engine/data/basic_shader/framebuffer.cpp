#include "framebuffer.h"

namespace mite {
FrameBuffer::FrameBuffer(const FrameBufferSpec &spec) : m_Spec(spec)
{
  // 创建时立即初始化帧缓冲
  Invalidate();
}

FrameBuffer::~FrameBuffer()
{
  // 析构时释放资源
  Release();
}

void FrameBuffer::Invalidate()
{
  // 如果已有帧缓冲，先释放
  if (m_RendererID) {
    Release();
  }
  // 创建帧缓冲对象
  glCreateFramebuffers(1, &m_RendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  const bool multisample = m_Spec.samples > 1;
  std::vector<GLenum> colorAttachments;  // Color存在多处AttacheMent点，单独计数

  // 附件配置结构
  struct AttachmentConfig {
    GLenum attachmentPoint;
    GLenum minFilter;
    GLenum magFilter;
    bool generateMipmaps;
    bool isColorAttachment;
  };
  for (size_t i = 0; i < m_Spec.attachments.size(); ++i) {
    const auto &attachmentSpec = m_Spec.attachments[i];
    AttachmentConfig config;
    // 单一switch处理所有参数配置
    switch (attachmentSpec.type) {
      case FrameBufferAttachmentType::Color:
        config.attachmentPoint = GL_COLOR_ATTACHMENT0 +
                                 static_cast<GLenum>(colorAttachments.size());
        config.minFilter = attachmentSpec.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        config.magFilter = GL_LINEAR;
        config.generateMipmaps = attachmentSpec.generateMipmaps &&
                                 !multisample;  // 多采样时不生成mipmaps
        config.isColorAttachment = true;
        break;
      case FrameBufferAttachmentType::Depth:
        config.attachmentPoint = GL_DEPTH_ATTACHMENT;
        config.minFilter = GL_NEAREST;  // 深度/模板使用最邻近过滤，避免插值导致的深度精度问题
        config.magFilter = GL_NEAREST;
        config.generateMipmaps = false;  // 深度/模板不生成Mipmap
        config.isColorAttachment = false;
        break;
      case FrameBufferAttachmentType::DepthStencil:
        config.attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
        config.minFilter = GL_NEAREST;
        config.magFilter = GL_NEAREST;
        config.generateMipmaps = false;
        config.isColorAttachment = false;
        break;
      case FrameBufferAttachmentType::Stencil:
        config.attachmentPoint = GL_STENCIL_ATTACHMENT;
        config.minFilter = GL_NEAREST;
        config.magFilter = GL_NEAREST;
        config.generateMipmaps = false;
        config.isColorAttachment = false;
        break;
      default:
        LOG_WARN("Unsupported framebuffer attachment type!");
        break;  // 跳过不支持的附件类型
    }
    // 统一的纹理创建和配置逻辑
    uint32_t textureID;
    glCreateTextures(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &textureID);
    if (multisample) {
      glTextureStorage2DMultisample(textureID,
                                    m_Spec.samples,
                                    static_cast<GLenum>(attachmentSpec.internalFormat),
                                    m_Spec.width,
                                    m_Spec.height,
                                    GL_FALSE);
    }
    else {
      glTextureStorage2D(textureID,
                         1,
                         static_cast<GLenum>(attachmentSpec.internalFormat),
                         m_Spec.width,
                         m_Spec.height);
      // 设置纹理参数
      glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, config.minFilter);
      glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, config.magFilter);
      glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      // 生成mipmaps（如果配置需要且非多采样）
      if (config.generateMipmaps) {
        glGenerateTextureMipmap(textureID);
      }
    }
    // 附加到帧缓冲
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           config.attachmentPoint,
                           multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                           textureID,
                           0);
    // 保存附件
    m_Attachments[static_cast<uint32_t>(i)] = textureID;
    // 记录颜色附件
    if (config.isColorAttachment) {
      colorAttachments.push_back(config.attachmentPoint);
    }
  }
  // 设置绘制缓冲区
  if (!colorAttachments.empty()) {
    glDrawBuffers(static_cast<GLsizei>(colorAttachments.size()), colorAttachments.data());
  }
  else {
    // 只有深度/模板缓冲时，显式告诉OpenGL不渲染到任何颜色缓冲
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
  // 检查帧缓冲完整性
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    LOG_ERROR("Framebuffer is incomplete!");
  }
  // 解绑帧缓冲
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Release()
{
  // 删除颜色附件纹理
  for (auto &[index, textureID] : m_Attachments) {
    glDeleteTextures(1, &textureID);
  }
  m_Attachments.clear();

  // 删除帧缓冲对象
  if (m_RendererID) {
    glDeleteFramebuffers(1, &m_RendererID);
    m_RendererID = 0;
  }
}

void FrameBuffer::Resize(uint32_t width, uint32_t height)
{
  // 检查尺寸是否有效
  if (width == 0 || height == 0) {
    LOG_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
    return;
  }

  // 更新规格并重新初始化
  m_Spec.width = width;
  m_Spec.height = height;
  Invalidate();
}

void FrameBuffer::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

  // 设置视口大小匹配帧缓冲大小
  glViewport(0, 0, m_Spec.width, m_Spec.height);
}

void FrameBuffer::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t FrameBuffer::GetID() const
{
  return m_RendererID;
}

uint32_t FrameBuffer::GetColorAttachmentID(uint32_t index) const
{
  if (m_Attachments.find(index) != m_Attachments.end()) {
    return m_Attachments.at(index);
  }
  LOG_WARN("Color attachment at index {0} not found!", index);
  return 0;
}

bool FrameBuffer::IsComplete() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return status == GL_FRAMEBUFFER_COMPLETE;
}
}  // namespace mite