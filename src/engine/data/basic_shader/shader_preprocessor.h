#ifndef MITE_SHADER_PREPROCESSOR_H
#define MITE_SHADER_PREPROCESSOR_H

#include "headers/headers.h"

namespace mite {

/**
 * @brief 着色器预处理器 - 处理#include指令和路径解析
 * @note 所有路径都通过FileSystem::GetAssetPath解析为绝对路径
 * @note 支持头文件防护（#ifndef/#define/#endif）
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
    m_definedGuards.clear();
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
  /**
   * @brief 处理头文件防护逻辑
   * @param source 原始源码
   * @param assetPath 文件路径（用于生成防护标识符）
   * @return 处理后的源码
   */
  std::string ProcessHeaderGuards(const std::string &source, const std::string &assetPath);
  /**
   * @brief 检查是否是头文件防护开始
   * @param line 代码行
   * @return 如果是防护开始返回true
   */
  bool IsGuardStart(const std::string &line, std::string &guardName) const;
  /**
   * @brief 检查是否是头文件防护定义
   * @param line 代码行
   * @param guardName 防护名称
   * @return 如果是防护定义返回true
   */
  bool IsGuardDefine(const std::string &line, const std::string &guardName) const;
  /**
   * @brief 检查是否是头文件防护结束
   * @param line 代码行
   * @return 如果是防护结束返回true
   */
  bool IsGuardEnd(const std::string &line) const;
  /**
   * @brief 生成唯一的防护标识符
   * @param assetPath 文件路径
   * @return 防护标识符
   */
  std::string GenerateGuardName(const std::string &assetPath) const;

 private:
  std::unordered_set<std::string> m_includedFiles;  // 已包含文件集合，避免重复包含
  std::unordered_set<std::string> m_definedGuards;  // 已定义的防护标识符
  static const int MAX_INCLUDE_DEPTH = 16;          // 最大包含深度，防止无限递归
};

}  // namespace mite

#endif  // MITE_SHADER_PREPROCESSOR_H
