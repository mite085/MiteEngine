#include "shader.h"

namespace mite {
OpenGLShader::OpenGLShader() {}

OpenGLShader::~OpenGLShader()
{
  Destroy();
}

void OpenGLShader::LoadFromFile(const char *vertexPath,
                          const char *fragmentPath,
                          const char *geometryPath)
{
  // 1. 从文件读取着色器源码
  std::string vertexCode, fragmentCode, geometryCode;
  std::ifstream vShaderFile, fShaderFile, gShaderFile;

  // 确保ifstream对象能抛出异常
  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // 打开文件并读取到字符串流
    
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    // 可选几何着色器
    if (geometryPath != nullptr) {
      gShaderFile.open(geometryPath);
      std::stringstream gShaderStream;
      gShaderStream << gShaderFile.rdbuf();
      gShaderFile.close();
      geometryCode = gShaderStream.str();
    }
  }
  catch (std::ifstream::failure &e) {
    LOG_CRITICAL("ERROR::SHADER::FILE_READ_FAILED {}", e.what());
    throw std::runtime_error("Shader file load failed");
  }

  // 2. 调用源码加载接口
  LoadFromSource(vertexCode, fragmentCode, geometryCode);
}

void OpenGLShader::LoadFromSource(const std::string &vertexSrc,
                            const std::string &fragmentSrc,
                            const std::string &geometrySrc)
{
  // 1. 编译着色器
  m_Handle.vertexShader = CompileShader(vertexSrc, GL_VERTEX_SHADER);
  m_Handle.fragmentShader = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
  m_Handle.geometryShader = 0;

  // 可选几何着色器
  if (!geometrySrc.empty()) {
    m_Handle.geometryShader = CompileShader(geometrySrc, GL_GEOMETRY_SHADER);
  }

  // 2. 创建着色器程序
  m_Handle.programId = glCreateProgram();
  glAttachShader(static_cast<GLuint>(m_Handle.programId),
                 static_cast<GLuint>(m_Handle.vertexShader));
  glAttachShader(static_cast<GLuint>(m_Handle.programId),
                 static_cast<GLuint>(m_Handle.fragmentShader));
  if (m_Handle.geometryShader != 0) {
    glAttachShader(static_cast<GLuint>(m_Handle.programId),
                   static_cast<GLuint>(m_Handle.geometryShader));
  }

  // 3. 链接程序
  glLinkProgram(static_cast<GLuint>(m_Handle.programId));
  CheckCompileErrors(static_cast<GLuint>(m_Handle.programId), GL_LINK_STATUS, true);

  // 4. 删除临时着色器对象（已链接到程序中）
  glDeleteShader(static_cast<GLuint>(m_Handle.vertexShader));
  glDeleteShader(static_cast<GLuint>(m_Handle.fragmentShader));
  if (m_Handle.geometryShader != 0) {
    glDeleteShader(static_cast<GLuint>(m_Handle.geometryShader));
  }
}

void OpenGLShader::Destroy()
{
  if (m_Handle.programId != 0) {
    glDeleteProgram(static_cast<GLuint>(m_Handle.programId));
    m_Handle.programId = 0;
  }
  m_UniformLocationCache.clear();
}

// =============== Uniform设置方法 ===============
void OpenGLShader::SetBool(const std::string &name, bool value)
{
  glUniform1i(GetUniformLocation(name), (int)value);
}

void OpenGLShader::SetInt(const std::string &name, int value)
{
  glUniform1i(GetUniformLocation(name), value);
}

void OpenGLShader::SetFloat(const std::string &name, float value)
{
  glUniform1f(GetUniformLocation(name), value);
}

void OpenGLShader::SetVec2(const std::string &name, const glm::vec2 &value)
{
  glUniform2fv(GetUniformLocation(name), 1, &value[0]);
}

void OpenGLShader::SetVec3(const std::string &name, const glm::vec3 &value)
{
  glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void OpenGLShader::SetVec4(const std::string &name, const glm::vec4 &value)
{
  glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void OpenGLShader::SetMat3(const std::string &name, const glm::mat3 &mat)
{
  glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void OpenGLShader::SetMat4(const std::string &name, const glm::mat4 &mat)
{
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void OpenGLShader::SetIntArray(const std::string &name, const int *values, size_t count)
{
  if (count == 0 || values == nullptr) {
    LOG_WARN("Attempting to set empty int array for uniform: {}", name);
    return;
  }

  const int location = GetUniformLocation(name);
  if (location == -1)
    return;  // 已通过GetUniformLocation输出警告

  glUniform1iv(location, static_cast<GLsizei>(count), values);

// 4. OpenGL错误检查
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    LOG_ERROR("OpenGL error in SetIntArray({}): {}", name, err);
  }
}

void OpenGLShader::SetFloatArray(const std::string &name, const float *values, size_t count)
{
  // 1. 参数校验
  if (count == 0 || values == nullptr) {
    LOG_WARN("Attempting to set empty float array for uniform: {}", name);
    return;
  }

  // 2. 获取Uniform位置
  const int location = GetUniformLocation(name);
  if (location == -1) {
    return;  // 已通过GetUniformLocation输出警告
  }

  // 3. 调用OpenGL接口
  glUniform1fv(location, static_cast<GLsizei>(count), values);

// 4. OpenGL错误检查
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    LOG_ERROR("[OpenGL] SetFloatArray({}) failed with error 0x{:X}", name, err);
    // 附加错误解释
    switch (err) {
      case GL_INVALID_OPERATION:
        LOG_ERROR("  - Shader program not linked or not a valid program");
        break;
      case GL_INVALID_VALUE:
        LOG_ERROR("  - Location {} is invalid", location);
        break;
    }
  }
}

void OpenGLShader::SetVector3Array(const std::string &name, const glm::vec3 *values, size_t count)
{
  // 1. 参数校验
  if (count == 0 || values == nullptr) {
    LOG_WARN("Attempting to set empty vec3 array for uniform: {}", name);
    return;
  }

  // 2. 获取Uniform位置
  const int location = GetUniformLocation(name);
  if (location == -1) {
    return;  // 已通过GetUniformLocation输出警告
  }

  // 3. 调用OpenGL接口
  glUniform3fv(location, static_cast<GLsizei>(count), glm::value_ptr(values[0]));

// 4. OpenGL错误检查
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    LOG_ERROR("[OpenGL] SetVector3Array({}) failed with error 0x{:X}", name, err);
    // 附加调试信息
    LOG_DEBUG("  - Array count: {}, First element: ({}, {}, {})",
              count,
              values[0].x,
              values[0].y,
              values[0].z);
  }
}

void OpenGLShader::SetUniformBlockBinding(const std::string &uniformBlockName,
                                          uint32_t bindingPoint)
{
  uint32_t blockIndex = GetUniformBlockIndex(uniformBlockName);
  if (blockIndex != GL_INVALID_INDEX) {
    glUniformBlockBinding(static_cast<GLuint>(m_Handle.programId), blockIndex, bindingPoint);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      LOG_ERROR("Failed to set uniform block binding: {} -> {}, error: 0x{:X}",
                uniformBlockName,
                bindingPoint,
                error);
    }
    else {
      LOG_DEBUG("Uniform block '{}' bound to point {}", uniformBlockName, bindingPoint);
    }
  }
  else {
    LOG_WARN("Uniform block '{}' not found in shader", uniformBlockName);
  }
}

void OpenGLShader::SetShaderStorageBlockBinding(const std::string &storageBlockName,
                                                uint32_t bindingPoint)
{
  uint32_t blockIndex = GetShaderStorageBlockIndex(storageBlockName);
  if (blockIndex != GL_INVALID_INDEX) {
    glShaderStorageBlockBinding(static_cast<GLuint>(m_Handle.programId), blockIndex, bindingPoint);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      LOG_ERROR("Failed to set shader storage block binding: {} -> {}, error: 0x{:X}",
                storageBlockName,
                bindingPoint,
                error);
    }
    else {
      LOG_DEBUG("Shader storage block '{}' bound to point {}", storageBlockName, bindingPoint);
    }
  }
  else {
    LOG_WARN("Shader storage block '{}' not found in shader", storageBlockName);
  }
}

uint32_t OpenGLShader::GetUniformBlockIndex(const std::string &uniformBlockName) const
{
  // 检查缓存
  if (auto it = m_UniformBlockCache.find(uniformBlockName); it != m_UniformBlockCache.end()) {
    return it->second;
  }

  // 查询OpenGL
  uint32_t blockIndex = glGetUniformBlockIndex(static_cast<GLuint>(m_Handle.programId),
                                               uniformBlockName.c_str());

  m_UniformBlockCache[uniformBlockName] = blockIndex;

  if (blockIndex == GL_INVALID_INDEX) {
    LOG_TRACE("Uniform block '{}' not found in shader", uniformBlockName);
  }

  return blockIndex;
}

uint32_t OpenGLShader::GetShaderStorageBlockIndex(const std::string &storageBlockName) const
{
  // 检查缓存
  if (auto it = m_StorageBlockCache.find(storageBlockName); it != m_StorageBlockCache.end()) {
    return it->second;
  }

  // 查询OpenGL
  uint32_t blockIndex = glGetProgramResourceIndex(
      static_cast<GLuint>(m_Handle.programId), GL_SHADER_STORAGE_BLOCK, storageBlockName.c_str());

  m_StorageBlockCache[storageBlockName] = blockIndex;

  if (blockIndex == GL_INVALID_INDEX) {
    LOG_TRACE("Shader storage block '{}' not found in shader", storageBlockName);
  }

  return blockIndex;
}


// =============== 私有工具方法 ===============
uint32_t OpenGLShader::CompileShader(const std::string &source, uint32_t type)
{
  uint32_t id = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);
  CheckCompileErrors(id, GL_COMPILE_STATUS, false);
  return id;
}

void OpenGLShader::CheckCompileErrors(uint32_t id, uint32_t type, bool isProgram)
{
  int success;
  char infoLog[1024];

  if (isProgram) {
    glGetProgramiv(id, type, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, nullptr, infoLog);
      LOG_CRITICAL("ERROR::PROGRAM_LINK_ERROR: {}", infoLog);
    }
  }
  else {
    glGetShaderiv(id, type, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, nullptr, infoLog);
      LOG_CRITICAL("ERROR::SHADER_COMPILE_ERROR: {}", infoLog);
    }
  }
}

int OpenGLShader::GetUniformLocation(const std::string &name)
{
  // 检查缓存
  if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
    return m_UniformLocationCache[name];
  }

  // 查询OpenGL并缓存结果
  int location = glGetUniformLocation(static_cast<GLuint>(m_Handle.programId), name.c_str());
  if (location == -1) {
    LOG_CRITICAL("WARNING: Uniform {} not found in shader!", name);
  }
  m_UniformLocationCache[name] = location;
  return location;
}

void OpenGLShader::Bind() const
{
  glUseProgram(static_cast<GLuint>(m_Handle.programId));
}

void OpenGLShader::Unbind() const
{
  glUseProgram(0);
}
}  // namespace mite
