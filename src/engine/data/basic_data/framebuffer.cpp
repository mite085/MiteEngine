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

  // 处理多采样
  const bool multisample = m_Spec.samples > 1;

  // 创建附件
  for (size_t i = 0; i < m_Spec.attachments.size(); ++i) {
    const auto &attachmentSpec = m_Spec.attachments[i];

    switch (attachmentSpec.type) {
      case FrameBufferAttachmentType::Color: {
        // 创建颜色附件纹理
        uint32_t textureID;
        glCreateTextures(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &textureID);

        if (multisample) {
          glTextureStorage2DMultisample(textureID,
                                        m_Spec.samples,
                                        attachmentSpec.internalFormat,
                                        m_Spec.width,
                                        m_Spec.height,
                                        GL_FALSE);
        }
        else {
          glTextureStorage2D(
              textureID, 1, attachmentSpec.internalFormat, m_Spec.width, m_Spec.height);

          // 设置纹理参数(非多采样时才需要)
          glTextureParameteri(textureID,
                              GL_TEXTURE_MIN_FILTER,
                              attachmentSpec.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR :
                                                               GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

          // 如果需要生成mipmaps
          if (attachmentSpec.generateMipmaps) {
            glGenerateTextureMipmap(textureID);
          }
        }

        // 将纹理附加到帧缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                               multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                               textureID,
                               0);

        // 保存颜色附件
        m_ColorAttachments[static_cast<uint32_t>(i)] = textureID;
        break;
      }
      case FrameBufferAttachmentType::Depth: {
        // 创建深度附件纹理
        uint32_t textureID;
        glCreateTextures(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &textureID);

        if (multisample) {
          glTextureStorage2DMultisample(textureID,
                                        m_Spec.samples,
                                        GL_DEPTH_COMPONENT24,
                                        m_Spec.width,
                                        m_Spec.height,
                                        GL_FALSE);
        }
        else {
          glTextureStorage2D(textureID, 1, GL_DEPTH_COMPONENT24, m_Spec.width, m_Spec.height);

          // 设置纹理参数
          glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        // 将纹理附加到帧缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                               textureID,
                               0);

        // 保存深度附件
        m_DepthAttachment = textureID;
        break;
      }
      case FrameBufferAttachmentType::DepthStencil: {
        // 创建深度模板附件纹理
        uint32_t textureID;
        glCreateTextures(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &textureID);

        if (multisample) {
          glTextureStorage2DMultisample(textureID,
                                        m_Spec.samples,
                                        GL_DEPTH24_STENCIL8,
                                        m_Spec.width,
                                        m_Spec.height,
                                        GL_FALSE);
        }
        else {
          glTextureStorage2D(textureID, 1, GL_DEPTH24_STENCIL8, m_Spec.width, m_Spec.height);

          // 设置纹理参数
          glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        // 将纹理附加到帧缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_STENCIL_ATTACHMENT,
                               multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                               textureID,
                               0);

        // 保存深度附件(共用同一个纹理)
        m_DepthAttachment = textureID;
        break;
      }
      default:
        LOG_WARN("Unsupported framebuffer attachment type!");
        break;
    }
  }

  // 设置绘制缓冲区(颜色附件)
  if (!m_ColorAttachments.empty()) {
    std::vector<GLenum> buffers;
    buffers.reserve(m_ColorAttachments.size());
    for (const auto &[index, _] : m_ColorAttachments) {
      buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }
    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
  }
  else {
    // 只有深度缓冲时，显式告诉OpenGL我们不渲染到任何颜色缓冲
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
  for (auto &[index, textureID] : m_ColorAttachments) {
    glDeleteTextures(1, &textureID);
  }
  m_ColorAttachments.clear();

  // 删除深度附件纹理
  if (m_DepthAttachment) {
    glDeleteTextures(1, &m_DepthAttachment);
    m_DepthAttachment = 0;
  }

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
  if (m_ColorAttachments.find(index) != m_ColorAttachments.end()) {
    return m_ColorAttachments.at(index);
  }
  LOG_WARN("Color attachment at index {0} not found!", index);
  return 0;
}

uint32_t FrameBuffer::GetDepthAttachmentID() const
{
  return m_DepthAttachment;
}

bool FrameBuffer::IsComplete() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return status == GL_FRAMEBUFFER_COMPLETE;
}

}  // namespace mite