#include "shader_preprocessor.h"

namespace mite {
std::string ShaderPreprocessor::Preprocess(const std::string &source,
                                           const std::string &baseAssetPath)
{
  std::stringstream result;
  std::stringstream input(source);
  std::string line;

  while (std::getline(input, line)) {
    // 检查是否是#include指令
    if (line.find("#include") != std::string::npos) {
      try {
        std::string includedContent = ProcessInclude(line, baseAssetPath, 0);
        result << includedContent << "\n";
      }
      catch (const std::exception &e) {
        LOG_ERROR("Failed to process include in '{}': {}", baseAssetPath, e.what());
        // 保留原始行以便调试
        result << "// ERROR: " << line << " - " << e.what() << "\n";
      }
    }
    else {
      result << line << "\n";
    }
  }

  return result.str();
}
std::string ShaderPreprocessor::PreprocessFromAsset(const std::string &assetPath)
{
  // 读取文件内容
  std::string source = ReadAssetFile(assetPath);

  // 记录主文件
  m_includedFiles.insert(assetPath);

  LOG_DEBUG("Preprocessing shader: {}", assetPath);

  // 预处理
  return Preprocess(source, assetPath);
}
std::string ShaderPreprocessor::ProcessInclude(const std::string &line,
                                               const std::string &currentAssetPath,
                                               int depth)
{
  // 检查递归深度
  if (depth >= MAX_INCLUDE_DEPTH) {
    throw std::runtime_error("Include depth exceeded maximum limit of " +
                             std::to_string(MAX_INCLUDE_DEPTH));
  }

  // 解析包含路径
  std::string includeAssetPath = ResolveIncludeAssetPath(line, currentAssetPath);
  std::string normalizedPath = NormalizeAssetPath(includeAssetPath);

  // 检查是否已经包含
  if (IsAlreadyIncluded(normalizedPath)) {
    LOG_DEBUG("File already included, skipping: {}", normalizedPath);
    return "// Already included: " + normalizedPath + "\n";
  }

  // 标记为已包含
  m_includedFiles.insert(normalizedPath);

  // 读取包含文件
  std::string includedSource = ReadAssetFile(normalizedPath);

  LOG_DEBUG("Processing include: {} -> {} (depth: {})", currentAssetPath, normalizedPath, depth);

  // 递归处理嵌套包含
  return Preprocess(includedSource, normalizedPath);
}
std::string ShaderPreprocessor::ResolveIncludeAssetPath(const std::string &includeLine,
                                                        const std::string &currentAssetPath)
{
  // 使用正则表达式匹配 #include "path" 或 #include <path>
  std::regex includeRegex(R"(#include\s*[<"]([^">]+)[">])");
  std::smatch match;

  if (!std::regex_search(includeLine, match, includeRegex) || match.size() < 2) {
    throw std::runtime_error("Invalid include directive: " + includeLine);
  }

  std::string relativePath = match[1].str();
  std::string currentDir = GetAssetDirectory(currentAssetPath);

  // 构建完整的Asset路径
  std::filesystem::path fullPath = std::filesystem::path(currentDir) / relativePath;

  // 转换为字符串并规范化
  return NormalizeAssetPath(fullPath.string());
}
std::string ShaderPreprocessor::ReadAssetFile(const std::string &assetPath)
{
  std::filesystem::path absolutePath = FileSystem::GetAssetPath(assetPath);

  if (!FileSystem::Exists(absolutePath)) {
    throw std::runtime_error("Shader file not found: " + assetPath +
                             " (absolute: " + absolutePath.string() + ")");
  }

  return FileSystem::ReadFileToString(absolutePath);
}
bool ShaderPreprocessor::IsAlreadyIncluded(const std::string &assetPath) const
{
  return m_includedFiles.find(assetPath) != m_includedFiles.end();
}
std::string ShaderPreprocessor::GetAssetDirectory(const std::string &assetPath) const
{
  return std::filesystem::path(assetPath).parent_path().string();
}
std::string ShaderPreprocessor::NormalizeAssetPath(const std::string &assetPath) const
{
  // 使用filesystem的规范化功能
  std::filesystem::path path(assetPath);

  // 移除尾随的斜杠
  if (path.has_filename()) {
    return path.string();
  }
  else {
    // 如果是目录路径，确保以斜杠结尾（可选）
    return path.string();
  }
}

}  // namespace mite
