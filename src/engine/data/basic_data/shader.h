#ifndef MITE_DATA_SHADER
#define MITE_DATA_SHADER

#include "headers/headers.h"
#include "basic_type/handle_type.h"

namespace mite {
/**
 * @brief Shader程序封装类（管理顶点/片段/几何着色器的编译、链接和Uniform操作）
 * @note 线程安全性：Shader对象应在渲染线程创建和使用
 */
class OpenGLShader {
 public:
  OpenGLShader();
  ~OpenGLShader();

  // ---- 生命周期 ----
  /**
   * @brief 从源码文件加载并编译Shader
   * @param vertexPath   顶点着色器文件路径
   * @param fragmentPath 片段着色器文件路径
   * @param geometryPath 可选几何着色器路径（nullptr表示忽略）
   * @throws std::runtime_error 编译/链接失败时抛出异常
   */
  void LoadFromFile(const char *vertexPath,
                    const char *fragmentPath,
                    const char *geometryPath = nullptr);

  /**
   * @brief 直接使用源码字符串编译Shader（适用于内置Shader）
   * @param vertexSrc   顶点着色器源码
   * @param fragmentSrc 片段着色器源码
   */
  void LoadFromSource(const std::string &vertexSrc,
                      const std::string &fragmentSrc,
                      const std::string &geometrySrc);

  void Destroy();  // 显式释放GPU资源

  // ---- Uniform设置 ----
  void SetBool(const std::string &name, bool value);
  void SetInt(const std::string &name, int value);
  void SetFloat(const std::string &name, float value);
  void SetVec2(const std::string &name, const glm::vec2 &value);
  void SetVec3(const std::string &name, const glm::vec3 &value);
  void SetVec4(const std::string &name, const glm::vec4 &value);
  void SetMat3(const std::string &name, const glm::mat3 &mat);
  void SetMat4(const std::string &name, const glm::mat4 &mat);

  void SetIntArray(const std::string &name, const int *values, size_t count);
  void SetFloatArray(const std::string &name, const float *values, size_t count);
  void SetVector3Array(const std::string &name, const glm::vec3 *values, size_t count);

  // ---- 状态控制 ----
  void Bind() const;    // 绑定当前Shader为激活状态
  void Unbind() const;  // 解绑Shader

  // ---- 工具函数 ----
  ShaderGPUHandle GetHandle() const
  {
    return m_Handle;
  }

 private:
  // ---- 私有方法 ----
  /**
   * @brief 编译单个着色器阶段
   * @param source 着色器源码
   * @param type 着色器类型（GL_VERTEX_SHADER等）
   * @return 编译成功的着色器ID
   */
  uint32_t CompileShader(const std::string &source, uint32_t type);

  /**
   * @brief 检查着色器/程序的编译链接错误
   * @param id 着色器或程序ID
   * @param type 检查类型（GL_COMPILE_STATUS或GL_LINK_STATUS）
   * @param isProgram 是否为程序对象（true=程序，false=着色器）
   */
  void CheckCompileErrors(uint32_t id, uint32_t type, bool isProgram);

  /**
   * @brief
   * @param name
   * @return
   */
  int GetUniformLocation(const std::string &name);

  // ---- 成员变量 ----
  ShaderGPUHandle m_Handle;                                             // OpenGL程序GPU句柄
  mutable std::unordered_map<std::string, int> m_UniformLocationCache;  // Uniform位置缓存
};
}  // namespace mite

#endif
