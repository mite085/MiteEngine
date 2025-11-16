#ifndef MITE_SHADOW_INSTANCE_H
#define MITE_SHADOW_INSTANCE_H

#include "basic_instance/camera_instance.h"
#include "basic_shader/shader_ubo.h"
#include "light.h"
#include "shadow_map_type.h"

namespace mite {
/**
 * @brief 阴影实例类，负责管理阴影UBO的生命周期
 * @note 设计理念：
 * - 与相机、材质、网格系统保持一致的设计模式
 * - 负责阴影UBO的创建、更新和绑定
 * - 每帧更新所有光源的阴影数据
 */
class ShadowInstance {
 public:
  /**
   * @brief 构造函数
   */
  explicit ShadowInstance();
  ~ShadowInstance();

  // ==================== UBO管理接口 ====================
  /**
   * @brief 初始化阴影UBO
   * @return 是否初始化成功
   */
  bool InitializeUBO();
  /**
   * @brief 更新阴影UBO数据（每帧调用）
   * @param directionalLights 方向光源列表
   * @param pointLights 点光源列表
   * @param spotLights 聚光灯列表
   * @return 是否更新成功
   */
  bool UpdateUBO(const std::vector<std::shared_ptr<Light>> &lights,
                 const std::unordered_map<Light *, Transform> &lightTransforms,
                 std::shared_ptr<CameraInstance> cameraInstance);
  /**
   * @brief 绑定阴影UBO到当前渲染状态
   */
  void BindUBO();

  // ==================== 访问接口 ====================
  /**
   * @brief 获取UBO对象
   */
  std::shared_ptr<ShaderUBO> GetUBO() const { return m_ShadowUBO; }
  /**
   * @brief 获取当前阴影数据
   */
  const ShadowUniformBuffer &GetShadowData() const { return m_ShadowData; }

 private:
  bool ProcessDirectionalLight(std::shared_ptr<Light> light,
                               const Transform &lightTransform,
                               uint32_t lightIndex,
                               std::shared_ptr<CameraInstance> cameraInstance);
  bool ProcessPointLight(std::shared_ptr<Light> light,
                         const Transform &lightTransform,
                         uint32_t lightIndex,
                         std::shared_ptr<CameraInstance> cameraInstance);
  bool ProcessSpotLight(std::shared_ptr<Light> light,
                        const Transform &lightTransform,
                        uint32_t lightIndex,
                        std::shared_ptr<CameraInstance> cameraInstance);

  std::shared_ptr<ShaderUBO> m_ShadowUBO;  // 阴影UBO实例
  ShadowUniformBuffer m_ShadowData;        // 阴影数据缓存

  // 禁用拷贝构造和赋值
  ShadowInstance(const ShadowInstance &) = delete;
  ShadowInstance &operator=(const ShadowInstance &) = delete;
};
}  // namespace mite

#endif  // MITE_SHADOW_INSTANCE_H
