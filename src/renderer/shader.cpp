#include "shader.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后

namespace mite {
Shader::Shader() : m_RendererID(0) {}

Shader::~Shader()
{
  Destroy();
}

void Shader::LoadFromFile(const char *vertexPath,
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

void Shader::LoadFromSource(const std::string &vertexSrc,
                            const std::string &fragmentSrc,
                            const std::string &geometrySrc)
{
  // 1. 编译着色器
  uint32_t vertexID = CompileShader(vertexSrc, GL_VERTEX_SHADER);
  uint32_t fragmentID = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
  uint32_t geometryID = 0;

  // 可选几何着色器
  if (!geometrySrc.empty()) {
    geometryID = CompileShader(geometrySrc, GL_GEOMETRY_SHADER);
  }

  // 2. 创建着色器程序
  m_RendererID = glCreateProgram();
  glAttachShader(m_RendererID, vertexID);
  glAttachShader(m_RendererID, fragmentID);
  if (geometryID != 0) {
    glAttachShader(m_RendererID, geometryID);
  }

  // 3. 链接程序
  glLinkProgram(m_RendererID);
  CheckCompileErrors(m_RendererID, GL_LINK_STATUS, true);

  // 4. 删除临时着色器对象（已链接到程序中）
  glDeleteShader(vertexID);
  glDeleteShader(fragmentID);
  if (geometryID != 0) {
    glDeleteShader(geometryID);
  }
}

void Shader::Destroy()
{
  if (m_RendererID != 0) {
    glDeleteProgram(m_RendererID);
    m_RendererID = 0;
  }
  m_UniformLocationCache.clear();
}

// =============== Uniform设置方法 ===============
void Shader::SetBool(const std::string &name, bool value)
{
  glUniform1i(GetUniformLocation(name), (int)value);
}

void Shader::SetInt(const std::string &name, int value)
{
  glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string &name, float value)
{
  glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec3(const std::string &name, const glm::vec3 &value)
{
  glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetVec4(const std::string &name, const glm::vec4 &value)
{
  glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetMat3(const std::string &name, const glm::mat3 &mat)
{
  glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string &name, const glm::mat4 &mat)
{
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

// =============== 私有工具方法 ===============
uint32_t Shader::CompileShader(const std::string &source, uint32_t type)
{
  uint32_t id = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);
  CheckCompileErrors(id, GL_COMPILE_STATUS, false);
  return id;
}

void Shader::CheckCompileErrors(uint32_t id, uint32_t type, bool isProgram)
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

int Shader::GetUniformLocation(const std::string &name)
{
  // 检查缓存
  if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
    return m_UniformLocationCache[name];
  }

  // 查询OpenGL并缓存结果
  int location = glGetUniformLocation(m_RendererID, name.c_str());
  if (location == -1) {
    LOG_CRITICAL("WARNING: Uniform {} not found in shader!", name);
  }
  m_UniformLocationCache[name] = location;
  return location;
}

void Shader::Bind() const
{
  glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
  glUseProgram(0);
}
}  // namespace mite
