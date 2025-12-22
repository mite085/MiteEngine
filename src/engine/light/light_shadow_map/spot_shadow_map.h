#ifndef MITE_SPOT_SHADOW_MAP_H
#define MITE_SPOT_SHADOW_MAP_H

#include "light_core/shadow_map.h"

namespace mite {
/**
 * @brief 聚光灯阴影贴图类
 * @note 聚光灯使用透视投影阴影技术，基于光源位置和方向计算单个阴影矩阵
 * 使用透视投影模拟聚光灯的锥形照射范围，支持内外角控制
 */
class SpotShadowMap : public ShadowMap {
 public:
  /**
   * @brief 构造函数
   * @param data 阴影贴图数据配置
   */
  explicit SpotShadowMap(const ShadowMapData &data);

  /**
   * @brief 准备阴影数据
   * @param lightWorldTransform 光源的世界变换矩阵
   * @param cameraTransform 相机变换矩阵（聚光灯不使用）
   * @param cameraProj 相机投影矩阵（聚光灯不使用）
   * @return 更新后的阴影数据
   * @note 聚光灯阴影基于光源位置和方向计算单个透视投影矩阵
   */
  ShadowMapData PrepareShadowData(
      const uint32_t lightIndex, const Transform &lightWorldTransform,
      const Transform &cameraTransform,
      const glm::mat4 &cameraProj = glm::mat4(1.0f)) override;

  /**
   * @brief 获取阴影矩阵数量
   * @return 固定返回1，聚光灯使用单个阴影矩阵
   */
  size_t GetShadowMatrixCount() const override;

  /**
   * @brief 获取特定索引的阴影矩阵
   * @param index 矩阵索引（必须为0）
   * @return 对应的阴影视图投影矩阵
   */
  glm::mat4 GetShadowMatrix(size_t index) const override;

  /**
   * @brief 检查是否需要更新阴影数据
   * @return 是否需要更新
   * @note 当光源移动、旋转或配置改变时需要更新
   */
  bool NeedsUpdate() const override;

  /**
   * @brief 标记阴影数据已更新
   */
  void MarkUpdated() override;

  /**
   * @brief 获取阴影类型名称
   * @return 类型名称字符串 "SpotShadowMap"
   */
  std::string GetShadowTypeName() const override;

  // ---- 聚光灯阴影特定方法 ----

  /**
   * @brief 设置阴影范围
   * @param nearPlane 近平面距离
   * @param farPlane 远平面距离
   * @note 远平面通常设置为聚光灯的照射范围
   */
  void SetShadowRange(float nearPlane, float farPlane);

  /**
   * @brief 设置聚光灯视角参数
   * @param fov 视野角度，单位：度
   * @param aspectRatio 宽高比
   * @note 视野角度通常使用聚光灯的外角
   */
  void SetPerspectiveParams(float fov, float aspectRatio = 1.0f);

  /**
   * @brief 获取近平面距离
   * @return 近平面距离
   */
  float GetNearPlane() const;

  /**
   * @brief 获取远平面距离
   * @return 远平面距离
   */
  float GetFarPlane() const;

  /**
   * @brief 获取视野角度
   * @return 视野角度，单位：度
   */
  float GetFov() const;

  /**
   * @brief 获取宽高比
   * @return 宽高比
   */
  float GetAspectRatio() const;

 private:
  glm::vec3 m_LastLightPosition;   // 上一次计算时的光源位置，用于检测移动
  glm::vec3 m_LastLightDirection;  // 上一次计算时的光源方向，用于检测旋转
  float m_Fov;                     // 视野角度（度）
  float m_AspectRatio;             // 宽高比

  /**
   * @brief 计算聚光灯的视图投影矩阵
   * @param lightPosition 光源位置
   * @param lightDirection 光源方向
   * @note 基于光源位置和方向创建透视投影矩阵
   */
  void CalculateSpotLightMatrix(const glm::vec3 &lightPosition,
                                const glm::vec3 &lightDirection);

  /**
   * @brief 检查光源是否移动或旋转
   * @param newPosition 新的光源位置
   * @param newDirection 新的光源方向
   * @return 是否发生移动或旋转（超过阈值）
   */
  bool HasLightTransformChanged(const glm::vec3 &newPosition,
                                const glm::vec3 &newDirection) const;
};
}  // namespace mite

#endif  // MITE_SPOT_SHADOW_MAP_H
