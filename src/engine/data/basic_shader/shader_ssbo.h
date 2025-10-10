#ifndef MITE_SHADER_SSBO_H
#define MITE_SHADER_SSBO_H

#include "shader.h"
#include "uniform_buffer.h"

namespace mite {
/**
 * @brief 着色器存储缓冲区对象封装类
 * @note 职责：
 * 1. 管理OpenGL SSBO的创建、更新和销毁
 * 2. 支持GPU计算和大量数据存储
 * 3. 提供读写映射支持
 * 4. 支持原子操作和图像加载存储
 *
 * @note SSBO优势：
 * - 容量更大（通常几百MB到几GB）
 * - 支持读写操作
 * - 支持原子操作
 * - 支持可变长度数组
 * - 计算着色器友好
 */
class ShaderSSBO {
 public:
  /**
   * @brief 构造函数 - 使用预分配的绑定点
   */
  explicit ShaderSSBO(size_t size,
                      uint32_t bindingPoint,
                      GLenum usage = GL_DYNAMIC_DRAW);
  ~ShaderSSBO();

  // ---- 生命周期管理 ----
  void Initialize();
  void Destroy();
  bool IsInitialized() const
  {
    return m_IsInitialized;
  }

  // ---- 数据操作 ----
  /**
   * @brief 更新SSBO数据
   * @param data 数据指针
   * @param size 数据大小
   * @param offset 数据偏移量
   * @return 是否成功
   */
  bool UpdateData(const void *data, size_t size, size_t offset = 0);
  /**
   * @brief 从SSBO读取数据到CPU
   * @param data 目标缓冲区
   * @param size 读取大小
   * @param offset SSBO偏移量
   * @return 是否成功
   */
  bool ReadData(void *data, size_t size, size_t offset = 0) const;

  // ---- 内存映射支持 ----
  /**
   * @brief 映射SSBO到CPU内存
   * @param access 访问模式（GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE）
   * @return 映射的内存指针，失败返回nullptr
   */
  void *MapBuffer(GLenum access = GL_READ_WRITE);
  /**
   * @brief 解映射SSBO
   * @return 是否成功
   */
  bool UnmapBuffer();

  // ---- 绑定管理 ----
  void Bind() const;
  void Unbind() const;

  // ---- 工具方法 ----
  /**
   * @brief 为着色器设置SSBO绑定点
   * @param shader 目标着色器
   * @param storageBlockName 存储块名称
   * @param bindingPoint 绑定点
   */
  void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader,
                          const std::string &storageBlockName) const;
  /**
   * @brief 清除SSBO数据（填充0）
   * @param clearValue 清除值
   * @param offset 偏移量
   * @param size 清除大小
   * @return 是否成功
   */
  bool ClearData(uint32_t clearValue = 0, size_t offset = 0, size_t size = 0);

  // ---- 属性访问 ----
  uint32_t GetSSBOId() const
  {
    return m_SSBOId;
  }
  size_t GetSize() const
  {
    return m_Size;
  }
  GLenum GetUsage() const
  {
    return m_Usage;
  }
  bool IsMapped() const
  {
    return m_IsMapped;
  }
  uint32_t GetBindingPoint() const
  {
    return m_BindingPoint;
  }

 private:
  uint32_t m_SSBOId = 0;                 // OpenGL SSBO句柄
  size_t m_Size = 0;                     // SSBO大小（字节）
  GLenum m_Usage = GL_DYNAMIC_DRAW;      // 缓冲区使用模式
  uint32_t m_BindingPoint = UINT32_MAX;  // 绑定点（通过BindingPointManager分配）
  bool m_IsInitialized = false;          // 初始化状态
  bool m_IsMapped = false;               // 内存映射状态
  // ---- 内部方法 ----
  void CreateSSBO();
  bool ValidateDataSize(size_t size, size_t offset) const;
  bool ValidateAccess() const;
};
// ---- 类型定义 ----
using ShaderSSBOPtr = std::shared_ptr<ShaderSSBO>;
}  // namespace mite
#endif  // MITE_SHADER_SSBO_H
