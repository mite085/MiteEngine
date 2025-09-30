#ifndef MITE_POINT_LIGHT_H
#define MITE_POINT_LIGHT_H

#include "light_core/light.h"
#include "light_shadow_map/point_shadow_map.h"

namespace mite {
/**
 * @brief 点光源类
 * @note 点光源从光源位置向所有方向均匀发射光线
 * 支持物理正确的衰减计算和立方体贴图阴影
 */
class PointLight : public Light {
 public:
  PointLight();

  // ---- 基类方法重写 ----
  std::string GetLightTypeName() const override;
  GPULightData PrepareGPULightData(const Transform &worldTransform) const override;
  float CalculateInfluenceRadius() const override;
  void CreateDefaultShadowMap() override;
  bool Validate() const override;

  // ---- 点光源特定属性访问器 ----
  /**
   * @brief 设置点光源影响半径
   * @param radius 影响半径，单位：米
   */
  void SetRadius(float radius);
  /**
   * @brief 获取点光源影响半径
   * @return 影响半径，单位：米
   */
  float GetRadius() const;
  /**
   * @brief 设置衰减系数
   * @param falloff 衰减系数，影响光照衰减曲线
   */
  void SetFalloff(float falloff);
  /**
   * @brief 获取衰减系数
   * @return 衰减系数值
   */
  float GetFalloff() const;
  /**
   * @brief 设置衰减模式
   * @param attenuation 衰减模式枚举
   */
  void SetAttenuation(LightAttenuation attenuation);
  /**
   * @brief 获取衰减模式
   * @return 当前衰减模式
   */
  LightAttenuation GetAttenuation() const;

 private:
  /**
   * @brief 验证点光源特定参数
   * @return 参数是否有效
   */
  bool ValidatePointLightParameters() const;
};

}  // namespace mite

#endif  // MITE_POINT_LIGHT_H
