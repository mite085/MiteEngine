#ifndef MITE_DIRECTIONAL_LIGHT_H
#define MITE_DIRECTIONAL_LIGHT_H

#include "light_core/light.h"
#include "light_shadow_map/directional_shadow_map.h"

namespace mite {
/**
 * @brief 点光源类
 * @note 点光源从光源位置向所有方向均匀发射光线
 * 支持物理正确的衰减计算和立方体贴图阴影
 */
class DirectionalLight : public Light {
 public:
  DirectionalLight();

  // ---- 基类方法重写 ----
  std::string GetLightTypeName() const override;
  GPULightData PrepareGPULightData(const Transform &worldTransform) const override;
  float CalculateInfluenceRadius() const override;
  void CreateDefaultShadowMap() override;
  bool Validate() const override;

  // ---- 点光源特定属性访问器 ----
  /**
   * @brief 设置方向光辐照度
   * @param radius 辐照度，单位：瓦/平米(W/m2)
   */
  void SetIrradius(float radius);
  /**
   * @brief 获取方向光辐照度
   * @return 辐照度，单位：瓦/平米(W/m2)
   */
  float GetIrradius() const;

 private:
  /**
   * @brief 验证方向光特定参数
   * @return 参数是否有效
   */
  bool ValidateDirectionalLightParameters() const;
};

}  // namespace mite

#endif  // MITE_POINT_LIGHT_H
