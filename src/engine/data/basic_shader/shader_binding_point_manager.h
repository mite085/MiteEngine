#ifndef MITE_BINDING_POINT_MANAGER_H
#define MITE_BINDING_POINT_MANAGER_H

#include "headers/headers.h"

namespace mite {

/**
 * @brief 统一绑定点管理器
 * @note 管理所有UBO、SSBO、纹理等资源的绑定点分配
 */
class BindingPointManager {
 public:
  // ---- 资源类型枚举 ----
  enum class ResourceType {
    // UBO类型
    CameraUBO = 0,  // 相机参数
    MaterialUBO,    // 材质参数
    SceneUBO,       // 场景全局参数

    // SSBO类型
    LightSSBO,     // 光源数据
    InstanceSSBO,  // 实例数据（待扩展）
    BoneSSBO,      // 骨骼动画数据（待扩展）

    // 纹理类型
    ShadowMap,       // 阴影贴图
    EnvironmentMap,  // 环境贴图（待扩展）
    BRDFLUT,         // BRDF查找表（待扩展）

    // 特殊用途
    ComputeSSBO,  // 计算着色器存储（待扩展）
    CustomUBO,    // 自定义UBO（待扩展）
    CustomSSBO,   // 自定义SSBO（待扩展）

    Count  // 类型总数
  };

  // ---- 绑定点范围定义 ----
  struct BindingRanges {
    // UBO范围: 0-15 (16个绑定点)
    static constexpr uint32_t UBO_START = 0;
    static constexpr uint32_t UBO_COUNT = 16;

    // SSBO范围: 16-31 (16个绑定点)
    static constexpr uint32_t SSBO_START = 16;
    static constexpr uint32_t SSBO_COUNT = 16;

    // 纹理范围: 32-95 (64个绑定点)
    static constexpr uint32_t TEXTURE_START = 32;
    static constexpr uint32_t TEXTURE_COUNT = 64;

    // 总绑定点数
    static constexpr uint32_t TOTAL_BINDING_POINTS = 96;
  };

  // ---- 单例访问 ----
  static BindingPointManager &Get();

  // ---- 核心接口 ----
  /**
   * @brief 分配绑定点
   * @param type 资源类型
   * @param name 资源名称（用于调试和识别）
   * @return 分配的绑定点，如果分配失败返回UINT32_MAX
   */
  uint32_t AllocateBindingPoint(ResourceType type, const std::string &name = "");

  /**
   * @brief 释放绑定点
   * @param bindingPoint 要释放的绑定点
   */
  void ReleaseBindingPoint(uint32_t bindingPoint);

  /**
   * @brief 预分配常用资源绑定点
   * @note 在引擎初始化时调用，确保常用资源有固定绑定点
   */
  void PreallocateCommonResources();

  /**
   * @brief 获取资源名称（用于调试）
   * @param bindingPoint 绑定点
   * @return 资源名称，如果未分配返回空字符串
   */
  std::string GetResourceName(uint32_t bindingPoint) const;

  /**
   * @brief 获取资源类型
   * @param bindingPoint 绑定点
   * @return 资源类型
   */
  ResourceType GetResourceType(uint32_t bindingPoint) const;

  /**
   * @brief 检查绑定点是否已分配
   * @param bindingPoint 绑定点
   * @return 是否已分配
   */
  bool IsBindingPointAllocated(uint32_t bindingPoint) const;

  /**
   * @brief 获取已分配的绑定点数量
   * @return 已分配数量
   */
  size_t GetAllocatedCount() const;

  /**
   * @brief 重置所有绑定点（仅用于测试）
   */
  void Reset();

  // ---- 便捷方法：获取常用资源的固定绑定点 ----
  uint32_t GetCameraUBOBinding() const
  {
    return m_CameraUBOBinding;
  }
  uint32_t GetLightSSBOBinding() const
  {
    return m_LightSSBOBinding;
  }
  uint32_t GetShadowMapBinding() const
  {
    return m_ShadowMapBinding;
  }

 private:
  BindingPointManager();
  ~BindingPointManager() = default;

  // 禁止拷贝
  BindingPointManager(const BindingPointManager &) = delete;
  BindingPointManager &operator=(const BindingPointManager &) = delete;

  // ---- 内部方法 ----
  uint32_t GetRangeStart(ResourceType type) const;
  uint32_t GetRangeCount(ResourceType type) const;
  bool IsValidBindingPoint(uint32_t point) const;
  void ValidateRanges() const;

  // ---- 成员变量 ----
  mutable std::mutex m_Mutex;

  // 绑定点分配状态
  std::bitset<BindingRanges::TOTAL_BINDING_POINTS> m_AllocatedPoints;

  // 资源信息记录
  std::unordered_map<uint32_t, std::string> m_ResourceNames;
  std::unordered_map<uint32_t, ResourceType> m_ResourceTypes;

  // 常用资源的固定绑定点（预分配）
  uint32_t m_CameraUBOBinding = UINT32_MAX;
  uint32_t m_LightSSBOBinding = UINT32_MAX;
  uint32_t m_ShadowMapBinding = UINT32_MAX;

  // 各类型的下一个可用绑定点
  std::array<std::atomic<uint32_t>, static_cast<size_t>(ResourceType::Count)> m_NextBindingPoints;
};

}  // namespace mite

#endif  // MITE_BINDING_POINT_MANAGER_H
