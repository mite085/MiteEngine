#include "shader.h"

namespace mite {
OpenGLShader::OpenGLShader()
{
  // 配置为 Vulkan 目标环境，保持 SPIR-V 严格验证
  m_CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
  m_CompileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
  m_CompileOptions.SetGenerateDebugInfo();

  // 添加 include 解析回调
  m_CompileOptions.SetIncluder(std::make_unique<ShaderIncluder>());

  // 添加预定义宏
  m_CompileOptions.AddMacroDefinition("GLSL_VERSION", "460");
  m_CompileOptions.AddMacroDefinition("VULKAN_TARGET", "1");
}
OpenGLShader::~OpenGLShader()
{
  Destroy();
}


void OpenGLShader::LoadFromFile(const char *vertexPath,
                                const char *fragmentPath,
                                const char *geometryPath)
{
  try {
    LOG_DEBUG("Compiling shader from files: vertex={}, fragment={}, geometry={}",
              vertexPath,
              fragmentPath,
              geometryPath ? geometryPath : "none");
    // 使用Shaderc编译文件到SPIR-V
    auto vertexSpirv = CompileFileToSPIRV(vertexPath, shaderc_vertex_shader);
    auto fragmentSpirv = CompileFileToSPIRV(fragmentPath, shaderc_fragment_shader);

    std::vector<uint32_t> geometrySpirv;
    if (geometryPath != nullptr) {
      geometrySpirv = CompileFileToSPIRV(geometryPath, shaderc_geometry_shader);
    }
    // 从SPIR-V加载着色器程序
    LoadFromSPIRV(vertexSpirv, fragmentSpirv, geometrySpirv);

    LOG_DEBUG("Shader compilation completed successfully (ID: {})", m_Handle.programId);
  }
  catch (std::exception &e) {
    LOG_CRITICAL("ERROR::SHADER::COMPILATION_FAILED: {}", e.what());
    throw std::runtime_error("Shader compilation failed: " + std::string(e.what()));
  }
}

void OpenGLShader::LoadFromSource(const std::string &vertexSrc,
                                  const std::string &fragmentSrc,
                                  const std::string &geometrySrc)
{
  try {
    LOG_DEBUG("Compiling shader from source strings");
    // 使用Shaderc编译源码到SPIR-V
    auto vertexSpirv = CompileGLSLToSPIRV(vertexSrc, "vertex_shader", shaderc_vertex_shader);
    auto fragmentSpirv = CompileGLSLToSPIRV(
        fragmentSrc, "fragment_shader", shaderc_fragment_shader);

    std::vector<uint32_t> geometrySpirv;
    if (!geometrySrc.empty()) {
      geometrySpirv = CompileGLSLToSPIRV(geometrySrc, "geometry_shader", shaderc_geometry_shader);
    }
    // 从SPIR-V加载着色器程序
    LoadFromSPIRV(vertexSpirv, fragmentSpirv, geometrySpirv);

    LOG_DEBUG("Shader compilation from source completed successfully (ID: {})",
              m_Handle.programId);
  }
  catch (std::exception &e) {
    LOG_CRITICAL("ERROR::SHADER::COMPILATION_FAILED: {}", e.what());
    throw std::runtime_error("Shader compilation failed: " + std::string(e.what()));
  }
}

std::vector<uint32_t> OpenGLShader::CompileGLSLToSPIRV(const std::string &source,
                                                       const std::string &filename,
                                                       shaderc_shader_kind kind)
{
  try {
    // 编译GLSL到SPIR-V（自动处理#include和#ifdef等预处理指令）
    shaderc::SpvCompilationResult result = m_Compiler.CompileGlslToSpv(
        source, kind, filename.c_str(), "main", m_CompileOptions);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
      std::string error_msg = result.GetErrorMessage();

      // 增强错误信息
      if (error_msg.find("include") != std::string::npos) {
        error_msg += "\nNote: Make sure include paths are relative to the current file";
      }

      throw std::runtime_error("Failed to compile " + filename + ":\n" + error_msg);
    }
    LOG_DEBUG("Successfully compiled {} to SPIR-V ({} words)",
              filename,
              result.cend() - result.cbegin());
    return {result.cbegin(), result.cend()};
  }
  catch (const std::exception &e) {
    // 重新抛出以保留调用栈
    throw std::runtime_error("Shader compilation error in " + filename + ": " + e.what());
  }
}

std::vector<uint32_t> OpenGLShader::CompileFileToSPIRV(const std::string &filename,
                                                       shaderc_shader_kind kind)
{
  // 读取文件内容
  std::filesystem::path absolutePath = FileSystem::GetAssetPath(filename);

  if (!FileSystem::Exists(absolutePath)) {
    throw std::runtime_error("Shader file not found: " + filename);
  }
  std::string source = FileSystem::ReadFileToString(absolutePath);
  return CompileGLSLToSPIRV(source, filename, kind);
}

uint32_t OpenGLShader::CompileSPIRVToGLShader(const std::vector<uint32_t> &spirv, uint32_t type)
{
  if (spirv.empty()) {
    throw std::runtime_error("Empty SPIR-V data provided");
  }
  uint32_t shader = glCreateShader(type);

  // 使用SPIR-V专用接口
  glShaderBinary(1,
                 &shader,
                 GL_SHADER_BINARY_FORMAT_SPIR_V,
                 spirv.data(),
                 static_cast<GLsizei>(spirv.size() * sizeof(uint32_t)));

  // 特殊化着色器（指定入口点）
  glSpecializeShader(shader, "main", 0, nullptr, nullptr);

  CheckCompileErrors(shader, GL_COMPILE_STATUS, false);
  return shader;
}

void OpenGLShader::LoadFromSPIRV(const std::vector<uint32_t> &vertexSpirv,
                                 const std::vector<uint32_t> &fragmentSpirv,
                                 const std::vector<uint32_t> &geometrySpirv)
{
  // 1. 编译SPIR-V到OpenGL着色器对象
  uint32_t vertexShader = CompileSPIRVToGLShader(vertexSpirv, GL_VERTEX_SHADER);
  uint32_t fragmentShader = CompileSPIRVToGLShader(fragmentSpirv, GL_FRAGMENT_SHADER);
  uint32_t geometryShader = 0;
  if (!geometrySpirv.empty()) {
    geometryShader = CompileSPIRVToGLShader(geometrySpirv, GL_GEOMETRY_SHADER);
  }
  // 2. 创建着色器程序
  m_Handle.programId = glCreateProgram();
  glAttachShader(static_cast<GLuint>(m_Handle.programId), vertexShader);
  glAttachShader(static_cast<GLuint>(m_Handle.programId), fragmentShader);
  if (geometryShader != 0) {
    glAttachShader(static_cast<GLuint>(m_Handle.programId), geometryShader);
  }
  // 3. 链接程序
  glLinkProgram(static_cast<GLuint>(m_Handle.programId));
  CheckCompileErrors(static_cast<GLuint>(m_Handle.programId), GL_LINK_STATUS, true);
  // 4. 清理临时着色器对象
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  if (geometryShader != 0) {
    glDeleteShader(geometryShader);
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

void OpenGLShader::SetUniformBlockBinding(const std::string &uniformBlockName,
                                          uint32_t bindingPoint)
{
  uint32_t blockIndex = GetUniformBlockIndex(uniformBlockName);
  if (blockIndex != GL_INVALID_INDEX) {
    glUniformBlockBinding(static_cast<GLuint>(m_Handle.programId), blockIndex, bindingPoint);
    LOG_DEBUG("Uniform block '{}' bound to point {}", uniformBlockName, bindingPoint);
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
    LOG_DEBUG("Shader storage block '{}' bound to point {}", storageBlockName, bindingPoint);
  }
  else {
    LOG_WARN("Shader storage block '{}' not found in shader", storageBlockName);
  }
}

uint32_t OpenGLShader::GetUniformBlockIndex(const std::string &uniformBlockName) const
{
  if (auto it = m_UniformBlockCache.find(uniformBlockName); it != m_UniformBlockCache.end()) {
    return it->second;
  }
  uint32_t blockIndex = glGetUniformBlockIndex(static_cast<GLuint>(m_Handle.programId),
                                               uniformBlockName.c_str());
  m_UniformBlockCache[uniformBlockName] = blockIndex;
  return blockIndex;
}

uint32_t OpenGLShader::GetShaderStorageBlockIndex(const std::string &storageBlockName) const
{
  if (auto it = m_StorageBlockCache.find(storageBlockName); it != m_StorageBlockCache.end()) {
    return it->second;
  }
  uint32_t blockIndex = glGetProgramResourceIndex(
      static_cast<GLuint>(m_Handle.programId), GL_SHADER_STORAGE_BLOCK, storageBlockName.c_str());
  m_StorageBlockCache[storageBlockName] = blockIndex;
  return blockIndex;
}

// =============== 私有工具方法 ===============
void OpenGLShader::CheckCompileErrors(uint32_t id, uint32_t type, bool isProgram)
{
  int success;
  char infoLog[1024];
  if (isProgram) {
    glGetProgramiv(id, type, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, nullptr, infoLog);
      LOG_CRITICAL("ERROR::PROGRAM_LINK_ERROR: {}", infoLog);
      throw std::runtime_error("Program linking failed: " + std::string(infoLog));
    }
  }
  else {
    glGetShaderiv(id, type, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, nullptr, infoLog);
      LOG_CRITICAL("ERROR::SHADER_COMPILE_ERROR: {}", infoLog);
      throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }
  }
}

// =============== 纹理绑定方法 ===============
void OpenGLShader::SetTextureBinding(const std::string &samplerName, uint32_t bindingPoint)
{
  int location = GetUniformLocation(samplerName);
  if (location != -1) {
    glUniform1i(location, static_cast<GLint>(bindingPoint));
    LOG_DEBUG("Texture sampler '{}' bound to point {}", samplerName, bindingPoint);
  }
  else {
    LOG_WARN("Texture sampler '{}' not found in shader", samplerName);
  }
}
int OpenGLShader::GetUniformLocation(const std::string &name) const
{
  // 检查缓存
  if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
    return m_UniformLocationCache[name];
  }
  // 查询OpenGL并缓存结果
  int location = glGetUniformLocation(static_cast<GLuint>(m_Handle.programId), name.c_str());
  if (location == -1) {
    LOG_TRACE("Uniform '{}' not found in shader (program ID: {})", name, m_Handle.programId);
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

void OpenGLShader::SetVulkanTarget(bool enable)
{
  if (enable) {
    m_CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan,
                                          shaderc_env_version_vulkan_1_2);
    m_CompileOptions.AddMacroDefinition("VULKAN_TARGET", "1");
  }
  else {
    m_CompileOptions.SetTargetEnvironment(shaderc_target_env_opengl_compat,
                                          shaderc_env_version_opengl_4_5);
    m_CompileOptions.AddMacroDefinition("VULKAN_TARGET", "0");
  }
}

shaderc_include_result *ShaderIncluder::GetInclude(const char *requested_source,
                                                   shaderc_include_type type,
                                                   const char *requesting_source,
                                                   size_t include_depth)
{
  std::string full_path;

  if (type == shaderc_include_type_relative) {
    // 相对路径：基于 requesting_source 的目录
    std::filesystem::path requesting_path(requesting_source);
    std::filesystem::path parent_dir = requesting_path.parent_path();
    full_path = (parent_dir / requested_source).string();
  }
  else {
    // 系统路径（通常不使用）
    full_path = requested_source;
  }

  // 规范化路径
  full_path = NormalizeAssetPath(full_path);

  // 检查循环包含
  if (m_includeHistory.find(full_path) != m_includeHistory.end()) {
    throw std::runtime_error("Circular include detected: " + full_path);
  }

  // 检查最大深度
  if (include_depth > 16) {
    throw std::runtime_error("Include depth exceeded maximum limit");
  }

  // 记录包含历史
  m_includeHistory.insert(full_path);

  // 使用 FileSystem 读取文件
  std::filesystem::path absolute_path = FileSystem::GetAssetPath(full_path);

  if (!FileSystem::Exists(absolute_path)) {
    throw std::runtime_error("Include file not found: " + full_path +
                             " (absolute: " + absolute_path.string() + ")");
  }

  std::string content = FileSystem::ReadFileToString(absolute_path);

  // 创建 include 结果
  auto result = new shaderc_include_result;
  // 需要持久化字符串数据
  auto *persistent_data = new PersistentIncludeData{content, full_path};

  result->content = persistent_data->content.c_str();
  result->content_length = persistent_data->content.length();
  result->source_name = persistent_data->source_name.c_str();
  result->source_name_length = persistent_data->source_name.length();
  result->user_data = persistent_data;

  return result;
}

void ShaderIncluder::ReleaseInclude(shaderc_include_result *data)
{
  if (data && data->user_data) {
    delete static_cast<PersistentIncludeData *>(data->user_data);
  }
  delete data;
}

std::string ShaderIncluder::NormalizeAssetPath(const std::string &path) const
{
  std::filesystem::path p(path);
  return std::filesystem::weakly_canonical(p).string();
}
}  // namespace mite