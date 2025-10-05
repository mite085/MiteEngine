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
  if (m_RendererID) {
    Release();
  }
  // 创建帧缓冲对象
  glCreateFramebuffers(1, &m_RendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

  const bool multisample = m_Spec.samples > 1;
  std::vector<GLenum> colorAttachments;
  // 清空现有附件
  m_ColorAttachments.clear();
  m_DepthAttachment = nullptr;
  m_StencilAttachment = nullptr;
  for (size_t i = 0; i < m_Spec.attachments.size(); ++i) {
    const auto &attachmentSpec = m_Spec.attachments[i];

    // 创建运行时纹理
    auto runtimeTexture = std::make_shared<RuntimeTexture>();

    if (!runtimeTexture->initialize(
            attachmentSpec.type, m_Spec.width, m_Spec.height, attachmentSpec.internalFormat))
    {
      LOG_ERROR("Failed to create runtime texture for framebuffer attachment type: {}",
                static_cast<int>(attachmentSpec.type));
      continue;
    }
    // 配置纹理参数和附件点
    TextureGPUHandle handle = runtimeTexture->getHandle();
    GLenum attachmentPoint = GL_NONE;
    GLuint handleID = static_cast<GLuint>(handle.apiHandle); 
    bool isColorAttachment = false;
    // 根据RuntimeTextureType确定OpenGL附件点和存储位置
    switch (attachmentSpec.type) {
      case RuntimeTextureType::Depth:
        attachmentPoint = GL_DEPTH_ATTACHMENT;
        m_DepthAttachment = runtimeTexture;

        // 深度附件使用最近邻过滤
        if (!multisample) {
          glTextureParameteri(handleID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTextureParameteri(handleID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        break;
      case RuntimeTextureType::Stencil:
        attachmentPoint = GL_STENCIL_ATTACHMENT;
        m_StencilAttachment = runtimeTexture;

        // 模板附件使用最近邻过滤
        if (!multisample) {
          glTextureParameteri(handleID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTextureParameteri(handleID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        break;
      default:
        // 其余类型都作为颜色附件处理
        attachmentPoint = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorAttachments.size());
        m_ColorAttachments[static_cast<uint32_t>(i)] = runtimeTexture;
        isColorAttachment = true;

        // 设置颜色附件的纹理参数
        if (!multisample) {
          GLenum minFilter = attachmentSpec.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
          glTextureParameteri(handleID, GL_TEXTURE_MIN_FILTER, minFilter);
          glTextureParameteri(handleID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTextureParameteri(handleID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

          if (attachmentSpec.generateMipmaps && !multisample) {
            glGenerateTextureMipmap(handleID);
          }
        }
        colorAttachments.push_back(attachmentPoint);
        break;
    }
    // 附加纹理到帧缓冲
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachmentPoint,
                           multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
                           handleID,
                           0);
    LOG_DEBUG("Created framebuffer attachment: type={}, format={}, size={}x{}",
              static_cast<int>(attachmentSpec.type),
              static_cast<int>(attachmentSpec.internalFormat),
              m_Spec.width,
              m_Spec.height);
  }
  // 设置绘制缓冲区
  if (!colorAttachments.empty()) {
    glDrawBuffers(static_cast<GLsizei>(colorAttachments.size()), colorAttachments.data());
  }
  else {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
  // 检查帧缓冲完整性
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    LOG_ERROR("Framebuffer is incomplete!");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Release()
{
  // 智能指针自动管理运行时纹理的生命周期
  m_ColorAttachments.clear();
  m_DepthAttachment = nullptr;
  m_StencilAttachment = nullptr;
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

RuntimeTexturePtr FrameBuffer::GetColorAttachment(uint32_t index) const
{
  if (m_ColorAttachments.find(index) != m_ColorAttachments.end()) {
    return m_ColorAttachments.at(index);
  }
  LOG_WARN("Color attachment at index {0} not found!", index);
  return 0;
}

RuntimeTexturePtr FrameBuffer::GetDepthAttachment() const
{
  return m_DepthAttachment;
}
RuntimeTexturePtr FrameBuffer::GetStencilAttachment() const
{
  return m_StencilAttachment;
}

bool FrameBuffer::IsComplete() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return status == GL_FRAMEBUFFER_COMPLETE;
}
}  // namespace mite