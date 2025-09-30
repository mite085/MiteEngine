#ifndef MITE_LIGHT_SSBO_H
#define MITE_LIGHT_SSBO_H

#include "basic_shader/shader.h"
#include "basic_shader/shader_ssbo.h"
#include "light_type.h"

namespace mite {

/**
 * @brief 光源数据SSBO管理器
 * @note 职责：
 * 1. 管理所有光源数据的GPU存储
 * 2. 提供光源数据的批量更新和同步
 * 3. 支持延迟渲染中的大量光源处理
 * 4. 自动处理光源数据的布局和更新
 *
 * 注意：项目初期以基本功能为主，避免过度设计
 */
class LightSSBO {
 public:
  /**
   * @brief 构造函数
   * @param maxLights 支持的最大光源数量，默认1024个
   */
  explicit LightSSBO(size_t maxLights = 1024);
  ~LightSSBO() = default;

  // ---- 生命周期管理 ----
  /**
   * @brief 初始化SSBO，分配GPU内存
   */
  void Initialize();

  /**
   * @brief 销毁SSBO，释放GPU资源
   */
  void Destroy();

  /**
   * @brief 检查SSBO是否已初始化
   * @return 初始化状态
   */
  bool IsInitialized() const;

  // ---- 数据管理 ----
  /**
   * @brief 批量更新所有光源数据到SSBO
   * @param lights 光源数据列表
   * @return 更新是否成功
   * @note 这是最常用的更新方式，性能最佳
   */
  bool UpdateLights(const std::vector<GPULightData> &lights);

  /**
   * @brief 更新单个光源数据
   * @param light 单个光源的GPU数据
   * @param index 光源在SSBO中的索引位置
   * @return 更新是否成功
   * @note 适用于动态更新单个光源的场景
   */
  bool UpdateLight(const GPULightData &light, size_t index);

  /**
   * @brief 清除所有光源数据
   * @return 清除是否成功
   * @note 将光源数量设为0，但不释放SSBO内存
   */
  bool ClearLights();

  // ---- 绑定管理 ----
  /**
   * @brief 绑定SSBO到指定绑定点
   * @param bindingPoint OpenGL绑定点索引
   */
  void Bind(uint32_t bindingPoint) const;

  /**
   * @brief 为着色器设置光源SSBO绑定点
   * @param shader 目标着色器对象
   * @param bindingPoint 绑定点索引
   */
  void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader, uint32_t bindingPoint) const;

  // ---- 统计信息 ----
  /**
   * @brief 获取支持的最大光源数量
   * @return 最大光源数
   */
  size_t GetMaxLights() const;

  /**
   * @brief 获取当前有效光源数量
   * @return 当前光源数
   */
  size_t GetCurrentLightCount() const;

  /**
   * @brief 获取SSBO总大小（字节）
   * @return SSBO大小
   */
  size_t GetSSBOSize() const;

  // ---- 配置 ----
  /**
   * @brief 设置最大光源数量（必须在初始化前调用）
   * @param maxLights 新的最大光源数
   */
  void SetMaxLights(size_t maxLights);

  /**
   * @brief 设置默认绑定点
   * @param bindingPoint 绑定点索引
   */
  void SetBindingPoint(uint32_t bindingPoint);

  /**
   * @brief 获取当前绑定点
   * @return 绑定点索引
   */
  uint32_t GetBindingPoint() const;

 private:
  // ---- 内部数据结构 ----
  /**
   * @brief SSBO中的数据内存布局
   * @note 采用简单的头部+数组布局，避免复杂结构
   */
  struct LightSSBOLayout {
    LightSSBOHeader header;     // 头部信息：光源数量
    GPULightData lights[1024];  // 光源数据数组：固定大小
  };

  // ---- 成员变量 ----
  std::unique_ptr<ShaderSSBO> m_SSBO;  // 底层SSBO对象，管理OpenGL资源
  size_t m_MaxLights;                  // 最大光源数量，决定SSBO大小
  size_t m_CurrentLightCount;          // 当前有效光源数量
  size_t m_SSBOSize;                   // SSBO总大小（字节）
  uint32_t m_BindingPoint = 0;         // 默认绑定点，通常为0
  bool m_IsInitialized = false;        // 初始化状态标志

  // ---- 内部工具方法 ----
  /**
   * @brief 计算SSBO所需的总内存大小
   * @return SSBO大小（字节）
   * @note 计算公式：头部大小 + 最大光源数 * 单个光源大小
   */
  size_t CalculateSSBOSize() const;

  /**
   * @brief 验证光源索引是否在有效范围内
   * @param index 要验证的光源索引
   * @return 索引是否有效
   */
  bool ValidateLightIndex(size_t index) const;

  /**
   * @brief 准备光源数据用于SSBO传输
   * @param lights 原始光源数据列表
   * @param header 输出的头部信息（包含实际光源数量）
   * @return 处理后的光源数据，确保不超过最大数量限制
   */
  std::vector<GPULightData> PrepareLightDataForSSBO(const std::vector<GPULightData> &lights,
                                                    LightSSBOHeader &header) const;

  /**
   * @brief 创建空的SSBO数据（用于初始化）
   * @return 空的光源数据列表
   */
  std::vector<GPULightData> CreateEmptyLightData() const;
};

// ---- 类型定义 ----
using LightSSBOPtr = std::shared_ptr<LightSSBO>;

}  // namespace mite

#endif  // MITE_LIGHT_SSBO_H
