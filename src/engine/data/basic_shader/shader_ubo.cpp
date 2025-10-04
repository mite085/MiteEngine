#include "shader_ubo.h"

namespace mite {

ShaderUBO::ShaderUBO(size_t size, GLenum usage) : m_Size(size), m_Usage(usage)
{
  // 验证大小合理性
  if (size == 0) {
    LOG_ERROR("ShaderUBO: Invalid size 0");
    throw std::invalid_argument("UBO size cannot be 0");
  }

  // 验证使用模式
  if (usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW && usage != GL_STREAM_DRAW) {
    LOG_WARN("ShaderUBO: Unusual usage mode 0x{:X}, using GL_DYNAMIC_DRAW", usage);
    m_Usage = GL_DYNAMIC_DRAW;
  }

  LOG_DEBUG("ShaderUBO created - Size: {} bytes, Usage: 0x{:X}", size, usage);
}

ShaderUBO::~ShaderUBO()
{
  Destroy();
}

void ShaderUBO::Initialize()
{
  if (m_IsInitialized) {
    LOG_WARN("ShaderUBO already initialized");
    return;
  }

  CreateUBO();
  m_IsInitialized = true;

  LOG_INFO("ShaderUBO initialized successfully - ID: {}, Size: {} bytes", m_UBOId, m_Size);
}

void ShaderUBO::Destroy()
{
  if (m_IsInitialized && m_UBOId != 0) {
    glDeleteBuffers(1, &m_UBOId);
    m_UBOId = 0;
    m_IsInitialized = false;

    LOG_DEBUG("ShaderUBO destroyed");
  }
}

void ShaderUBO::CreateUBO()
{
  // 生成UBO
  glGenBuffers(1, &m_UBOId);

  if (m_UBOId == 0) {
    LOG_ERROR("Failed to generate UBO: glGenBuffers returned 0");
    throw std::runtime_error("UBO generation failed");
  }

  // 绑定并分配内存
  glBindBuffer(GL_UNIFORM_BUFFER, m_UBOId);
  glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, m_Usage);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // 检查OpenGL错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to create UBO: OpenGL error 0x{:X}", error);
    glDeleteBuffers(1, &m_UBOId);
    m_UBOId = 0;
    throw std::runtime_error("UBO creation failed");
  }

  LOG_TRACE("UBO created - ID: {}, Size: {} bytes", m_UBOId, m_Size);
}

bool ShaderUBO::UpdateData(const void *data, size_t size, size_t offset)
{
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot update UBO data: not initialized");
    return false;
  }

  if (!data) {
    LOG_ERROR("Cannot update UBO data: null data pointer");
    return false;
  }

  if (!ValidateDataSize(size, offset)) {
    return false;
  }

  // 更新UBO数据
  glBindBuffer(GL_UNIFORM_BUFFER, m_UBOId);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // 检查错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to update UBO data: OpenGL error 0x{:X}", error);
    return false;
  }

  //LOG_TRACE("UBO data updated - ID: {}, Offset: {}, Size: {} bytes", m_UBOId, offset, size);
  return true;
}

bool ShaderUBO::ValidateDataSize(size_t size, size_t offset) const
{
  if (offset >= m_Size) {
    LOG_ERROR("UBO update offset {} exceeds UBO size {}", offset, m_Size);
    return false;
  }

  if (offset + size > m_Size) {
    LOG_ERROR("UBO update range [{}, {}) exceeds UBO size {}", offset, offset + size, m_Size);
    return false;
  }

  return true;
}

void ShaderUBO::Bind(uint32_t bindingPoint) const
{
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot bind UBO: not initialized");
    return;
  }
  glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_UBOId);
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR(
        "Failed to bind UBO {} to point {}: OpenGL error 0x{:X}", m_UBOId, bindingPoint, error);
  }
}

void ShaderUBO::Unbind(uint32_t bindingPoint) const
{
  glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, 0);
}

void ShaderUBO::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader,
                                   const std::string &uniformBlockName,
                                   uint32_t bindingPoint) const
{
  if (!shader) {
    LOG_ERROR("Cannot setup shader binding: null shader");
    return;
  }

  if (!m_IsInitialized) {
    LOG_ERROR("Cannot setup shader binding: UBO not initialized");
    return;
  }

  // 设置着色器的Uniform块绑定点
  shader->SetUniformBlockBinding(uniformBlockName, bindingPoint);

  LOG_DEBUG("Shader UBO binding setup - Block: '{}', Point: {}", uniformBlockName, bindingPoint);
}

}  // namespace mite
