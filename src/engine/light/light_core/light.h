#ifndef MITE_LIGHT_H
#define MITE_LIGHT_H

#include "basic_type/light_type.h"
#include "shadow_map.h"

namespace mite {
/**
 * @brief 光源抽象基类
 * @note 职责：
 * 1. 仅管理光源的光学属性
 * 2. 提供GPU数据准备接口
 * 3. 管理关联的阴影对象
 */
class Light {
 public:
  explicit Light(LightType type);
  virtual ~Light() = default;

  // ---- 类型标识 ----
  LightType GetType() const;
  virtual std::string GetLightTypeName() const = 0;

  // ---- 光学属性访问器 ----
  void SetColor(const glm::vec3 &color);
  const glm::vec3 &GetColor() const;
  void SetIntensity(float intensity);
  float GetIntensity() const;
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

  // ---- 完整属性访问 ----
  LightProperties &GetProperties();
  const LightProperties &GetProperties() const;
  void SetProperties(const LightProperties &props);

  // ---- 阴影管理 ----
  void SetShadowMap(ShadowMapPtr shadow);
  ShadowMapPtr GetShadowMap() const;

  /**
   * @brief 创建默认阴影贴图（根据光源类型）
   */
  virtual void CreateDefaultShadowMap() = 0;

  /**
   * @brief 检查是否投射阴影
   */
  bool IsCastingShadows() const;
  // ---- 核心抽象接口 ----

  /**
   * @brief 准备光源数据用于SSBO传输
   * @param worldTransform 光源世界变换（从场景图获取）
   * @return 构造好的GPU光源数据
   */
  virtual GPULightData PrepareGPULightData(const Transform &worldTransform) const = 0;
  /**
   * @brief 准备阴影数据
   * @param worldTransform 光源世界变换（从场景图获取）
   * @param cameraView 相机视图矩阵（用于级联计算）
   * @param cameraProj 相机投影矩阵
   * @return 阴影数据
   */
  ShadowMapData PrepareShadowData(const Transform &worldTransform,
                                  const Transform &cameraView,
                                  const glm::mat4 &cameraProj = glm::mat4(1.0f)) const;
  /**
   * @brief 计算光源的影响半径（用于粗略剔除）
   * @return 影响范围半径
   * @note 这只是粗略估计，精确范围检查由其他系统负责
   */
  virtual float CalculateInfluenceRadius() const = 0;
  /**
   * @brief 验证光源参数的有效性
   * @return 是否有效
   */
  virtual bool Validate() const;

 protected:
  LightType m_Type;
  LightProperties m_Properties;
  ShadowMapPtr m_ShadowMap;  // Light拥有ShadowMap的所有权
  // ---- 内部工具方法 ----
  bool ValidateBaseParameters() const;
};
}  // namespace mite

#endif  // MITE_LIGHT_H
