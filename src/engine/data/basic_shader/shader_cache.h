#ifndef MITE_DATA_SHADER_CACHE
#define MITE_DATA_SHADER_CACHE

#include "shader.h"

namespace mite {
/**
 * @brief Shader缓存管理器（单例模式）
 * @note 职责：
 * 1. 避免重复编译相同路径的Shader
 * 2. 自动管理Shader资源的生命周期（引用计数）
 * 3. 线程安全的缓存操作
 * 
 */
class ShaderCache {
 public:
  // 删除拷贝构造和赋值
  ShaderCache(const ShaderCache &) = delete;
  ShaderCache &operator=(const ShaderCache &) = delete;

  // 获取单例实例
  static ShaderCache &Get();

  /**
   * @brief 获取或创建Shader
   * @param vertexPath   顶点着色器文件路径
   * @param fragmentPath 片段着色器文件路径
   * @param geometryPath 可选几何着色器路径（空字符串表示忽略）
   * @return std::shared_ptr<Shader> 缓存的Shader智能指针
   * @throws std::runtime_error 着色器编译失败时抛出
   */
  std::shared_ptr<OpenGLShader> GetOpenGLShader(const std::string &vertexPath,
                              const std::string &fragmentPath,
                              const std::string &geometryPath = "");

  /**
   * @brief 清空所有缓存（强制释放GPU资源）
   * @note 仅应在渲染线程无操作时调用
   */
  void Clear();

 private:
  ShaderCache() = default;  // 禁止外部构造

  // 生成缓存键（保证相同着色器组合的唯一性）
  std::string GenerateCacheKey(const std::string &vertexPath,
                               const std::string &fragmentPath,
                               const std::string &geometryPath) const;

  std::mutex m_Mutex;
  std::unordered_map<std::string, std::weak_ptr<OpenGLShader>> m_Cache;
};

}  // namespace mite

#endif
