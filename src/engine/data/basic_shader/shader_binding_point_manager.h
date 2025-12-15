#ifndef MITE_BINDING_POINT_MANAGER_H
#define MITE_BINDING_POINT_MANAGER_H

#include "uniform_buffer.h"

namespace mite {
/**
 * @brief 统一绑定点管理器
 * @note 管理所有UBO、SSBO、纹理等资源的绑定点分配
 */
class BindingPointManager {
 public:
  // ---- 单例访问 ----
  static BindingPointManager &Get();

  // ---- 核心接口 ----
  /**
   * @brief 预分配常用资源绑定点
   * @note 在引擎初始化时调用，确保常用资源有固定绑定点
   */
  void PreallocateCommonResources();
  void Reset();

  // ---- UBO 绑定点接口 ----
  uint32_t AllocateUBOBinding(UBOResourceType type, const std::string &name = "");
  void ReleaseUBOBinding(uint32_t bindingPoint);
  uint32_t GetCameraUBOBinding() const { return m_CameraUBOBinding; }
  uint32_t GetMaterialUBOBinding() const { return m_MaterialUBOBinding; }
  uint32_t GetModelUBOBinding() const { return m_ModelUBOBinding; }
  uint32_t GetShadowUBOBinding() const { return m_ShadowUBOBinding; }
  uint32_t GetShadowRenderContextUBOBinding() const { return m_ShadowRenderContextUBOBinding; }

  // ---- SSBO 绑定点接口 ----
  uint32_t AllocateSSBOBinding(SSBOResourceType type, const std::string &name = "");
  void ReleaseSSBOBinding(uint32_t bindingPoint);
  uint32_t GetLightSSBOBinding() const { return m_LightSSBOBinding; }

  // ---- 纹理绑定点接口 ----
  uint32_t AllocateTextureBinding(TextureResourceType category, const std::string &name = "");
  void ReleaseTextureBinding(uint32_t textureUnit);
  uint32_t GetRuntimeTextureBinding(RuntimeTextureType type) const;
  uint32_t GetExternalTextureBinding(ExternalTextureType type) const;

  /**
   * @brief 获取所有已分配的纹理绑定点的数组
   * @return 包含所有已分配纹理绑定点的vector
   * @note 用于在渲染前绑定占位符纹理，避免OpenGL报错
   */
  std::vector<uint32_t> GetAllocatedTextureBindings() const;

  // ---- 查询接口 ----
  bool IsUBOBindingAllocated(uint32_t bindingPoint) const;
  bool IsSSBOBindingAllocated(uint32_t bindingPoint) const;
  bool IsTextureBindingAllocated(uint32_t textureUnit) const;

 private:
  BindingPointManager();
  ~BindingPointManager() = default;

  // 禁止拷贝
  BindingPointManager(const BindingPointManager &) = delete;
  BindingPointManager &operator=(const BindingPointManager &) = delete;

  // ---- 内部方法 ----
  /**
   * @brief 分配/释放绑定点
   * @param type 资源类型
   * @param name 资源名称（用于调试和识别）
   * @return 分配的绑定点，如果分配失败返回UINT32_MAX
   */
  uint32_t AllocateFromRange(std::bitset<1024> &allocated,
                             uint32_t rangeStart,
                             uint32_t rangeCount,
                             std::atomic<uint32_t> &nextPoint,
                             const std::string &name);
  void ReleaseFromRange(std::bitset<1024> &allocated, uint32_t point);

  // ---- 成员变量 ----
  mutable std::mutex m_Mutex;

  // UBO 绑定点管理
  std::bitset<1024> m_AllocatedUBOs;
  std::array<std::atomic<uint32_t>, static_cast<size_t>(UBOResourceType::Count)> m_NextUBOPoints;
  uint32_t m_CameraUBOBinding = UINT32_MAX;
  uint32_t m_MaterialUBOBinding = UINT32_MAX;
  uint32_t m_ModelUBOBinding = UINT32_MAX;
  uint32_t m_ShadowUBOBinding = UINT32_MAX;
  uint32_t m_ShadowRenderContextUBOBinding = UINT32_MAX;

  // SSBO 绑定点管理
  std::bitset<1024> m_AllocatedSSBOs;
  std::array<std::atomic<uint32_t>, static_cast<size_t>(SSBOResourceType::Count)> m_NextSSBOPoints;
  uint32_t m_LightSSBOBinding = UINT32_MAX;

  // 纹理单元管理
  std::bitset<1024> m_AllocatedTextures;  // 负责密集式bindpoint分配管理
  std::array<std::atomic<uint32_t>, static_cast<size_t>(TextureResourceType::Count)>
      m_NextTexturePoints; 

  // 纹理类型映射
  std::unordered_map<RuntimeTextureType, uint32_t> m_RuntimeTextureBindings;
  std::unordered_map<ExternalTextureType, uint32_t> m_ExternalTextureBindings;
};
}  // namespace mite

#endif  // MITE_BINDING_POINT_MANAGER_H
