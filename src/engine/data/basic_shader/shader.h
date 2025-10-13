#ifndef MITE_DATA_SHADER
#define MITE_DATA_SHADER

#include "basic_type/handle_type.h"
#include <shaderc/shaderc.hpp>

namespace mite {
/**
 * @brief Shader程序封装类（管理顶点/片段/几何着色器的编译、链接和Uniform操作）
 * @note 线程安全性：Shader对象应在渲染线程创建和使用
 * @note 遵循SPIRV风格，不需要手动设置采样器 uniform
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
                      const std::string &geometrySrc = "");

  void Destroy();  // 显式释放GPU资源

  // ---- Uniform Buffer/ Storage Buffer设置 (使用固定的绑定点执行显示绑定，无需手动管理) ----
  ///**
  // * @brief 设置Uniform缓冲区对象绑定点
  // * @param uniformBlockName Uniform块名称
  // * @param bindingPoint 绑定点索引
  // */
  //void SetUniformBlockBinding(const std::string &uniformBlockName, uint32_t bindingPoint);
  ///**
  // * @brief 设置着色器存储缓冲区对象绑定点
  // * @param storageBlockName 存储块名称
  // * @param bindingPoint 绑定点索引
  // */
  //void SetShaderStorageBlockBinding(const std::string &storageBlockName, uint32_t bindingPoint);
  ///**
  // * @brief 获取Uniform块索引
  // * @param uniformBlockName Uniform块名称
  // * @return 块索引，如果不存在返回GL_INVALID_INDEX
  // */
  //uint32_t GetUniformBlockIndex(const std::string &uniformBlockName) const;
  ///**
  // * @brief 获取着色器存储块索引
  // * @param storageBlockName 存储块名称
  // * @return 块索引，如果不存在返回GL_INVALID_INDEX
  // */
  //uint32_t GetShaderStorageBlockIndex(const std::string &storageBlockName) const;

  // ---- 状态控制 ----
  void Bind() const;    // 绑定当前Shader为激活状态
  void Unbind() const;  // 解绑Shader

  // ---- 工具函数 ----
  ShaderGPUHandle GetHandle() const
  {
    return m_Handle;
  }
  uint32_t GetProgramId() const
  {
    return static_cast<uint32_t>(m_Handle.programId);
  }

  // ---- 编译选项设置 ----
  void SetVulkanTarget(bool enable);

 private:
  // ---- Shaderc编译方法 ----
  /**
   * @brief 编译GLSL到SPIRV（仅用于shaderc的调用）
   * @param source 着色器源码
   * @param type 着色器类型（GL_VERTEX_SHADER等）
   * @return 编译成功的着色器ID
   */

  std::vector<uint32_t> CompileGLSLToSPIRV(const std::string &source,
                                           const std::string &filename,
                                           shaderc_shader_kind kind);
  std::vector<uint32_t> CompileFileToSPIRV(const std::string &filename, shaderc_shader_kind kind);

  // ---- SPIR-V到OpenGL转换 ----
  uint32_t CompileSPIRVToGLShader(const std::vector<uint32_t> &spirv, uint32_t type);
  void LoadFromSPIRV(const std::vector<uint32_t> &vertexSpirv,
                     const std::vector<uint32_t> &fragmentSpirv,
                     const std::vector<uint32_t> &geometrySpirv = {});

  /**
   * @brief 检查着色器/程序的编译链接错误
   * @param id 着色器或程序ID
   * @param type 检查类型（GL_COMPILE_STATUS或GL_LINK_STATUS）
   * @param isProgram 是否为程序对象（true=程序，false=着色器）
   */
  void CheckCompileErrors(uint32_t id, uint32_t type, bool isProgram);

  // ---- 成员变量 ----
  ShaderGPUHandle m_Handle;                                               // OpenGL程序GPU句柄
  shaderc::Compiler m_Compiler;                                           // ShaderC编译器
  shaderc::CompileOptions m_CompileOptions;                               // ShaderC编译选项
  //mutable std::unordered_map<std::string, uint32_t> m_UniformBlockCache;  // Uniform区块缓存
  //mutable std::unordered_map<std::string, uint32_t> m_StorageBlockCache;  // Storage区块缓存
};

//
/**
 * @brief 自定义 include 解析器
 * @note 负责读取并解析shader中的include，支持递归和相对路径
 */
class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
 public:
  shaderc_include_result *GetInclude(const char *requested_source,
                                     shaderc_include_type type,
                                     const char *requesting_source,
                                     size_t include_depth) override;

  void ReleaseInclude(shaderc_include_result *data) override;

 private:
  std::string NormalizeAssetPath(const std::string &path) const;

  struct PersistentIncludeData {
    std::string content;
    std::string source_name;
  };
};
}  // namespace mite

#endif
