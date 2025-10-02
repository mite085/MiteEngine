#ifndef MITE_SHADER_PREPROCESSOR_H
#define MITE_SHADER_PREPROCESSOR_H

#include "headers/headers.h"

namespace mite {

/**
 * @brief 着色器预处理器 - 处理#include指令和路径解析
 * @note 所有路径都通过FileSystem::GetAssetPath解析为绝对路径
 */
class ShaderPreprocessor {
 public:
  ShaderPreprocessor() = default;

  /**
   * @brief 预处理着色器源码
   * @param source 原始着色器源码
   * @param baseAssetPath 基础Asset路径（如"shaders/gbuffer/gbuffer.vert.glsl"）
   * @return 预处理后的完整源码
   */
  std::string Preprocess(const std::string &source, const std::string &baseAssetPath);

  /**
   * @brief 从Asset文件加载并预处理着色器
   * @param assetPath Asset相对路径（如"shaders/gbuffer/gbuffer.vert.glsl"）
   * @return 预处理后的完整源码
   */
  std::string PreprocessFromAsset(const std::string &assetPath);

  /**
   * @brief 清空包含历史（用于处理新的着色器）
   */
  void ClearIncludeHistory()
  {
    m_includedFiles.clear();
  }

 private:
  /**
   * @brief 处理单个#include指令
   * @param line 包含指令行
   * @param currentAssetPath 当前文件的Asset路径
   * @param depth 包含深度（用于防止无限递归）
   * @return 包含文件的内容
   */
  std::string ProcessInclude(const std::string &line,
                             const std::string &currentAssetPath,
                             int depth = 0);

  /**
   * @brief 解析包含文件的Asset路径
   * @param includeLine 包含指令行
   * @param currentAssetPath 当前文件的Asset路径
   * @return 解析出的Asset路径
   */
  std::string ResolveIncludeAssetPath(const std::string &includeLine,
                                      const std::string &currentAssetPath);

  /**
   * @brief 从Asset路径读取文件内容
   * @param assetPath Asset相对路径
   * @return 文件内容
   */
  std::string ReadAssetFile(const std::string &assetPath);

  /**
   * @brief 获取文件所在目录的Asset路径
   * @param assetPath 文件的Asset路径
   * @return 目录的Asset路径
   */
  std::string GetAssetDirectory(const std::string &assetPath) const;

  /**
   * @brief 规范化Asset路径（移除多余的./和../）
   * @param assetPath 原始Asset路径
   * @return 规范化后的Asset路径
   */
  std::string NormalizeAssetPath(const std::string &assetPath) const;

  /**
   * @brief 判断是否已经包含过该文件
   * @param assetPath 资产路径
   */
  bool IsAlreadyIncluded(const std::string &assetPath) const;

 private:
  std::unordered_set<std::string> m_includedFiles;  // 已包含文件集合，避免重复包含
  static const int MAX_INCLUDE_DEPTH = 16;          // 最大包含深度，防止无限递归
};

}  // namespace mite

#endif  // MITE_SHADER_PREPROCESSOR_H
