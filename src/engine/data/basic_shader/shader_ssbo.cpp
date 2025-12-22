#include "shader_ssbo.h"

#include "shader_binding_point_manager.h"

namespace mite {
ShaderSSBO::ShaderSSBO(size_t size, uint32_t bindingPoint, GLenum usage)
    : m_Size(size), m_Usage(usage), m_BindingPoint(bindingPoint) {
  if (size == 0) {
    LOG_ERROR("ShaderSSBO: Invalid size 0");
    throw std::invalid_argument("SSBO size cannot be 0");
  }
  // 验证使用模式
  if (usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW &&
      usage != GL_STREAM_DRAW && usage != GL_DYNAMIC_COPY &&
      usage != GL_DYNAMIC_READ) {
    LOG_WARN("ShaderSSBO: Unusual usage mode 0x{:X}, using GL_DYNAMIC_DRAW",
             usage);
    m_Usage = GL_DYNAMIC_DRAW;
  }
  LOG_DEBUG(
      "ShaderSSBO created with fixed binding - Size: {} bytes, Binding: {}",
      size, m_BindingPoint);
}
ShaderSSBO::~ShaderSSBO() {
  // 确保在销毁前解映射
  if (m_IsMapped) {
    LOG_WARN("ShaderSSBO destroyed while still mapped, forcing unmap");
    UnmapBuffer();
  }
  Destroy();
}

void ShaderSSBO::Initialize() {
  if (m_IsInitialized) {
    LOG_WARN("ShaderSSBO already initialized");
    return;
  }
  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Cannot initialize SSBO: no binding point allocated");
    throw std::runtime_error("SSBO has no binding point");
  }
  CreateSSBO();
  m_IsInitialized = true;
  LOG_INFO("ShaderSSBO initialized - ID: {}, Binding: {}", m_SSBOId,
           m_BindingPoint);
}

void ShaderSSBO::Destroy() {
  if (m_IsInitialized && m_SSBOId != 0) {
    // 确保在销毁前解映射
    if (m_IsMapped) {
      LOG_WARN("ShaderSSBO destroyed while still mapped, forcing unmap");
      UnmapBuffer();
    }

    glDeleteBuffers(1, &m_SSBOId);
    m_SSBOId = 0;
    m_IsInitialized = false;

    LOG_DEBUG("ShaderSSBO destroyed");
  }
}

void ShaderSSBO::CreateSSBO() {
  // 生成SSBO
  glGenBuffers(1, &m_SSBOId);

  if (m_SSBOId == 0) {
    LOG_ERROR("Failed to generate SSBO: glGenBuffers returned 0");
    throw std::runtime_error("SSBO generation failed");
  }

  // 绑定并分配内存
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_Size, nullptr, m_Usage);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // 检查OpenGL错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to create SSBO: OpenGL error {}", error);
    glDeleteBuffers(1, &m_SSBOId);
    m_SSBOId = 0;
    throw std::runtime_error("SSBO creation failed");
  }

  LOG_TRACE("SSBO created - ID: {}, Size: {} bytes", m_SSBOId, m_Size);
}

bool ShaderSSBO::UpdateData(const void *data, size_t size, size_t offset) {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot update SSBO data: not initialized");
    return false;
  }

  if (!data) {
    LOG_ERROR("Cannot update SSBO data: null data pointer");
    return false;
  }

  if (m_IsMapped) {
    LOG_ERROR("Cannot update SSBO data: buffer is currently mapped");
    return false;
  }

  if (!ValidateDataSize(size, offset)) {
    return false;
  }

  // 更新SSBO数据
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // 检查错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to update SSBO data: OpenGL error 0x{:X}", error);
    return false;
  }

  LOG_TRACE("SSBO data updated - ID: {}, Offset: {}, Size: {} bytes", m_SSBOId,
            offset, size);
  return true;
}

bool ShaderSSBO::ReadData(void *data, size_t size, size_t offset) const {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot read SSBO data: not initialized");
    return false;
  }

  if (!data) {
    LOG_ERROR("Cannot read SSBO data: null data pointer");
    return false;
  }

  if (m_IsMapped) {
    LOG_ERROR("Cannot read SSBO data: buffer is currently mapped");
    return false;
  }

  if (!ValidateDataSize(size, offset)) {
    return false;
  }

  // 从SSBO读取数据
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // 检查错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to read SSBO data: OpenGL error 0x{:X}", error);
    return false;
  }

  LOG_TRACE("SSBO data read - ID: {}, Offset: {}, Size: {} bytes", m_SSBOId,
            offset, size);
  return true;
}

void *ShaderSSBO::MapBuffer(GLenum access) {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot map SSBO: not initialized");
    return nullptr;
  }

  if (m_IsMapped) {
    LOG_ERROR("Cannot map SSBO: already mapped");
    return nullptr;
  }

  // 验证访问模式
  if (access != GL_READ_ONLY && access != GL_WRITE_ONLY &&
      access != GL_READ_WRITE) {
    LOG_ERROR("Invalid buffer mapping access mode: 0x{:X}", access);
    return nullptr;
  }

  // 映射缓冲区
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
  void *mappedPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  if (!mappedPtr) {
    LOG_ERROR("Failed to map SSBO: glMapBuffer returned nullptr");
    return nullptr;
  }

  m_IsMapped = true;
  LOG_DEBUG("SSBO mapped successfully - ID: {}, Access: 0x{:X}, Pointer: {}",
            m_SSBOId, access, reinterpret_cast<uintptr_t>(mappedPtr));

  return mappedPtr;
}

bool ShaderSSBO::UnmapBuffer() {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot unmap SSBO: not initialized");
    return false;
  }

  if (!m_IsMapped) {
    LOG_WARN("Cannot unmap SSBO: not currently mapped");
    return false;
  }

  // 解映射缓冲区
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOId);
  GLboolean result = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  if (result == GL_FALSE) {
    LOG_ERROR("Failed to unmap SSBO: glUnmapBuffer returned GL_FALSE");
    return false;
  }

  m_IsMapped = false;
  LOG_DEBUG("SSBO unmapped successfully - ID: {}", m_SSBOId);

  return true;
}

void ShaderSSBO::Bind() const {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot bind SSBO: not initialized");
    return;
  }
  if (m_IsMapped) {
    LOG_ERROR("Cannot bind SSBO: buffer is currently mapped");
    return;
  }
  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Cannot bind SSBO: no binding point allocated");
    return;
  }
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_SSBOId);
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOG_ERROR("Failed to bind SSBO to point {}: OpenGL error 0x{:X}",
              m_BindingPoint, error);
  } else {
    LOG_TRACE("SSBO bound to binding point: {}", m_BindingPoint);
  }
}

void ShaderSSBO::Unbind() const {
  if (m_BindingPoint != UINT32_MAX) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, 0);
  }
}

// void ShaderSSBO::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader,
//                                     const std::string &storageBlockName)
//                                     const
//{
//   if (!shader) {
//     LOG_ERROR("Cannot setup shader binding: null shader");
//     return;
//   }
//   if (!m_IsInitialized) {
//     LOG_ERROR("Cannot setup shader binding: SSBO not initialized");
//     return;
//   }
//   if (m_IsMapped) {
//     LOG_ERROR("Cannot setup shader binding: SSBO is currently mapped");
//     return;
//   }
//   if (m_BindingPoint == UINT32_MAX) {
//     LOG_ERROR("Cannot setup shader binding: SSBO has no binding point");
//     return;
//   }
//   // 设置着色器的存储块绑定点
//   shader->SetShaderStorageBlockBinding(storageBlockName, m_BindingPoint);
//   LOG_DEBUG(
//       "Shader SSBO binding setup - Block: '{}', Point: {}", storageBlockName,
//       m_BindingPoint);
// }

bool ShaderSSBO::ClearData(uint32_t clearValue, size_t offset, size_t size) {
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot clear SSBO data: not initialized");
    return false;
  }

  if (m_IsMapped) {
    LOG_ERROR("Cannot clear SSBO data: buffer is currently mapped");
    return false;
  }

  // 如果size为0，则清除整个缓冲区
  if (size == 0) {
    size = m_Size - offset;
  }

  if (!ValidateDataSize(size, offset)) {
    return false;
  }

  // 使用映射方式清除数据（更高效）
  void *mappedPtr = MapBuffer(GL_WRITE_ONLY);
  if (!mappedPtr) {
    return false;
  }

  // 清除指定区域
  uint32_t *dataPtr = static_cast<uint32_t *>(mappedPtr);
  size_t elementCount = size / sizeof(uint32_t);

  for (size_t i = 0; i < elementCount; ++i) {
    dataPtr[(offset / sizeof(uint32_t)) + i] = clearValue;
  }

  // 解映射
  if (!UnmapBuffer()) {
    LOG_ERROR("Failed to unmap SSBO after clear operation");
    return false;
  }

  LOG_DEBUG(
      "SSBO data cleared - ID: {}, Offset: {}, Size: {} bytes, Value: 0x{:X}",
      m_SSBOId, offset, size, clearValue);

  return true;
}

bool ShaderSSBO::ValidateDataSize(size_t size, size_t offset) const {
  if (offset >= m_Size) {
    LOG_ERROR("SSBO operation offset {} exceeds SSBO size {}", offset, m_Size);
    return false;
  }

  if (offset + size > m_Size) {
    LOG_ERROR("SSBO operation range [{}, {}) exceeds SSBO size {}", offset,
              offset + size, m_Size);
    return false;
  }

  return true;
}

bool ShaderSSBO::ValidateAccess() const {
  if (m_IsMapped) {
    LOG_ERROR("SSBO operation not allowed: buffer is currently mapped");
    return false;
  }

  return true;
}
}  // namespace mite