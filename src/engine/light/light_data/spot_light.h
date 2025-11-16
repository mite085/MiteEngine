#ifndef MITE_SPOT_LIGHT_H
#define MITE_SPOT_LIGHT_H

#include "light_core/light.h"
#include "light_shadow_map/spot_shadow_map.h"

namespace mite {
/**
 * @brief 聚光灯类
 * @note 聚光灯从光源位置向特定方向发射锥形光线
 * 支持内外角控制、边缘柔化和阴影投射
 */
class SpotLight : public Light {
 public:
  SpotLight();

  // ---- 基类方法重写 ----
  std::string GetLightTypeName() const override;
  GPULightData PrepareGPULightData(const Transform &worldTransform) const override;
  float CalculateInfluenceRadius() const override;
  void CreateDefaultShadowMap() override;
  bool Validate() const override;

  // ---- 聚光灯特定属性访问器 ----
  /**
   * @brief 设置聚光灯内角
   * @param angle 内角角度，单位：度
   */
  void SetInnerAngle(float angle);
  /**
   * @brief 获取聚光灯内角
   * @return 内角角度，单位：度
   */
  float GetInnerAngle() const;
  /**
   * @brief 设置聚光灯外角
   * @param angle 外角角度，单位：度
   */
  void SetOuterAngle(float angle);
  /**
   * @brief 获取聚光灯外角
   * @return 外角角度，单位：度
   */
  float GetOuterAngle() const;
  /**
   * @brief 设置边缘柔化系数
   * @param blend 柔化系数(0-1)
   */
  void SetBlend(float blend);
  /**
   * @brief 获取边缘柔化系数
   * @return 柔化系数值
   */
  float GetBlend() const;
  /**
   * @brief 设置照射范围
   * @param range 照射范围，单位：米
   */
  void SetRange(float range);
  /**
   * @brief 获取照射范围
   * @return 照射范围，单位：米
   */
  float GetRange() const;

 private:
  /**
   * @brief 验证聚光灯特定参数
   * @return 参数是否有效
   */
  bool ValidateSpotLightParameters() const;
};

}  // namespace mite

#endif  // MITE_SPOT_LIGHT_H
