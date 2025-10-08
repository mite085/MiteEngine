#ifndef MITE_SHADER_UBO_H
#define MITE_SHADER_UBO_H

#include "shader.h"
#include "uniform_buffer.h"

namespace mite {
/**
 * @brief Uniform缓冲区对象封装类
 * @note 职责：
 * 1. 管理OpenGL UBO的创建、更新和销毁
 * 2. 提供类型安全的UBO数据管理
 * 3. 支持动态和静态UBO数据更新
 * 4. 自动处理绑定点管理
 */
class ShaderUBO {
 public:
  /**
   * @brief 构造函数 - 使用预分配的绑定点
   * @param size UBO大小（字节）
   * @param bindingPoint 预分配的绑定点（使用BindingPointManager）
   * @param usage 缓冲区使用模式
   */
  explicit ShaderUBO(size_t size,
                     uint32_t bindingPoint,
                     GLenum usage = GL_DYNAMIC_DRAW);
  ~ShaderUBO();

  // ---- 生命周期管理 ----
  void Initialize();
  void Destroy();
  bool IsInitialized() const
  {
    return m_IsInitialized;
  }

  // ---- 数据操作 ----
  /**
   * @brief 更新整个UBO数据
   * @param data 数据指针
   * @param size 数据大小（必须<=创建时的大小）
   * @param offset 数据偏移量
   * @return 是否成功
   */
  bool UpdateData(const void *data, size_t size, size_t offset = 0);

  // ---- 绑定管理 ----
  /**
   * @brief 绑定UBO到指定绑定点
   * @param bindingPoint 绑定点索引
   */
  void Bind() const;
  /**
   * @brief 解绑UBO
   */
  void Unbind() const;

  // ---- 工具方法 ----
  /**
   * @brief 为着色器设置UBO绑定点
   * @param shader 目标着色器
   * @param uniformBlockName Uniform块名称
   */
  void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader,
                          const std::string &uniformBlockName) const;

  // ---- 属性访问 ----
  uint32_t GetUBOId() const
  {
    return m_UBOId;
  }
  size_t GetSize() const
  {
    return m_Size;
  }
  GLenum GetUsage() const
  {
    return m_Usage;
  }
  uint32_t GetBindingPoint() const
  {
    return m_BindingPoint;
  }

 private:
  uint32_t m_UBOId = 0;                  // OpenGL UBO句柄
  size_t m_Size = 0;                     // UBO大小（字节）
  GLenum m_Usage = GL_DYNAMIC_DRAW;      // 缓冲区使用模式
  uint32_t m_BindingPoint = UINT32_MAX;  // 绑定点（通过BindingPointManager分配）
  bool m_IsInitialized = false;          // 初始化状态

  // ---- 内部方法 ----
  void CreateUBO();
  bool ValidateDataSize(size_t size, size_t offset) const;
};

// ---- 类型定义 ----
using ShaderUBOPtr = std::shared_ptr<ShaderUBO>;
}  // namespace mite

#endif  // MITE_SHADER_UBO_H
