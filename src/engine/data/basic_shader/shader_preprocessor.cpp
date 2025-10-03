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

  // 预处理头文件防护
  std::string processedSource = ProcessHeaderGuards(source, assetPath);

  // 预处理包含指令
  return Preprocess(processedSource, assetPath);
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

  // 预处理头文件防护
  std::string processedSource = ProcessHeaderGuards(includedSource, normalizedPath);

  // 递归处理嵌套包含
  return Preprocess(processedSource, normalizedPath);
}

std::string ShaderPreprocessor::ProcessHeaderGuards(const std::string &source,
                                                    const std::string &assetPath)
{
  std::stringstream result;
  std::stringstream input(source);
  std::string line;

  bool inGuardBlock = false;
  bool guardDefined = false;
  std::string currentGuardName;
  int guardDepth = 0;

  while (std::getline(input, line)) {
    std::string trimmedLine = line;
    trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));  // 去除前导空白

    // 检查是否是头文件防护开始
    if (!inGuardBlock && IsGuardStart(trimmedLine, currentGuardName)) {
      inGuardBlock = true;
      guardDepth = 1;

      // 检查防护是否已经定义
      if (m_definedGuards.find(currentGuardName) != m_definedGuards.end()) {
        // 防护已定义，跳过整个块
        result << "// Guard skipped: " << currentGuardName << " (already defined)\n";
        continue;
      }
      else {
        // 防护未定义，保留#ifndef行
        result << line << "\n";
      }
      continue;
    }

    // 在防护块中
    if (inGuardBlock) {
      // 检查是否是防护定义
      if (!guardDefined && IsGuardDefine(trimmedLine, currentGuardName)) {
        guardDefined = true;
        m_definedGuards.insert(currentGuardName);
        result << line << "\n";
        continue;
      }

      // 检查是否是防护结束
      if (IsGuardEnd(trimmedLine)) {
        guardDepth--;
        if (guardDepth == 0) {
          inGuardBlock = false;
          guardDefined = false;
          result << line << "\n";
          continue;
        }
      }

      // 检查嵌套的#ifndef
      std::string nestedGuardName;
      if (IsGuardStart(trimmedLine, nestedGuardName)) {
        guardDepth++;
      }

      // 检查嵌套的#endif
      if (IsGuardEnd(trimmedLine)) {
        // 已经在上面处理过了
      }

      // 如果防护已定义，跳过内容
      if (m_definedGuards.find(currentGuardName) != m_definedGuards.end()) {
        // 跳过内容，但保留嵌套的防护指令
        if (trimmedLine.find("#ifndef") == 0 || trimmedLine.find("#endif") == 0) {
          result << line << "\n";
        }
        else {
          result << "// " << line << " (skipped due to guard)\n";
        }
      }
      else {
        // 保留内容
        result << line << "\n";
      }
    }
    else {
      // 不在防护块中，直接输出
      result << line << "\n";
    }
  }

  return result.str();
}

bool ShaderPreprocessor::IsGuardStart(const std::string &line, std::string &guardName) const
{
  std::regex guardRegex(R"(^#ifndef\s+(\w+)$)");
  std::smatch match;

  if (std::regex_match(line, match, guardRegex) && match.size() > 1) {
    guardName = match[1].str();
    return true;
  }
  return false;
}

bool ShaderPreprocessor::IsGuardDefine(const std::string &line, const std::string &guardName) const
{
  std::regex defineRegex(R"(^#define\s+(\w+)$)");
  std::smatch match;

  if (std::regex_match(line, match, defineRegex) && match.size() > 1) {
    return match[1].str() == guardName;
  }
  return false;
}

bool ShaderPreprocessor::IsGuardEnd(const std::string &line) const
{
  return line.find("#endif") == 0;
}

std::string ShaderPreprocessor::GenerateGuardName(const std::string &assetPath) const
{
  // 将路径转换为有效的C标识符
  std::string guardName = assetPath;
  std::replace(guardName.begin(), guardName.end(), '/', '_');
  std::replace(guardName.begin(), guardName.end(), '.', '_');
  std::replace(guardName.begin(), guardName.end(), ' ', '_');

  // 移除非法字符
  guardName.erase(std::remove_if(guardName.begin(),
                                 guardName.end(),
                                 [](char c) { return !std::isalnum(c) && c != '_'; }),
                  guardName.end());

  // 转换为大写
  std::transform(guardName.begin(), guardName.end(), guardName.begin(), ::toupper);

  return guardName + "_GLSL";
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
