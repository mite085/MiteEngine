#ifndef MITE_DIRECTIONAL_SHADOW_MAP_H
#define MITE_DIRECTIONAL_SHADOW_MAP_H

#include "light_core/shadow_map.h"

namespace mite {

/**
 * @brief 方向光阴影贴图类
 * @note 方向光使用级联阴影映射技术(CSM)，将视锥体分割为多个级联
 * 每个级联使用正交投影，提供不同距离的阴影质量优化
 */
class DirectionalShadowMap : public ShadowMap {
 public:
  /**
   * @brief 构造函数
   * @param data 阴影贴图数据配置
   */
  explicit DirectionalShadowMap(const ShadowMapData &data);

  /**
   * @brief 准备阴影数据
   * @param lightWorldTransform 光源的世界变换矩阵
   * @param cameraView 相机视图矩阵
   * @param cameraProj 相机投影矩阵
   * @return 更新后的阴影数据
   * @note 方向光阴影基于相机视锥体和光源方向计算多个级联的阴影矩阵
   */
  ShadowMapData PrepareShadowData(const uint32_t lightIndex,
                                  const Transform &lightWorldTransform,
                                  const Transform &cameraView,
                                  const glm::mat4 &cameraProj = glm::mat4(1.0f)) override;

  /**
   * @brief 获取阴影矩阵数量
   * @return 级联数量
   */
  size_t GetShadowMatrixCount() const override;

  /**
   * @brief 获取特定索引的阴影矩阵
   * @param index 矩阵索引（对应级联索引）
   * @return 对应的阴影视图投影矩阵
   */
  glm::mat4 GetShadowMatrix(size_t index) const override;

  /**
   * @brief 检查是否需要更新阴影数据
   * @return 是否需要更新
   * @note 当相机或光源移动、旋转或配置改变时需要更新
   */
  bool NeedsUpdate() const override;

  /**
   * @brief 标记阴影数据已更新
   */
  void MarkUpdated() override;

  /**
   * @brief 获取阴影类型名称
   * @return 类型名称字符串 "DirectionalShadowMap"
   */
  std::string GetShadowTypeName() const override;

  // ---- 方向光阴影特定方法 ----

  /**
   * @brief 设置级联参数
   * @param cascadeCount 级联数量（1-4）
   * @param splitLambda 分割参数（0-1，控制级联分布）
   */
  void SetCascadeParams(unsigned int cascadeCount, float splitLambda);

  /**
   * @brief 设置级联分割距离
   * @param splits 分割距离数组
   * @note 分割距离应该按升序排列，表示每个级联的远平面距离
   */
  void SetCascadeSplits(const std::array<float, 5> &splits);

  /**
   * @brief 获取级联数量
   * @return 当前级联数量
   */
  unsigned int GetCascadeCount() const;

  /**
   * @brief 获取分割参数
   * @return 分割参数值
   */
  float GetSplitLambda() const;

  /**
   * @brief 获取级联分割距离
   * @return 分割距离数组
   */
  const std::array<float, 5> &GetCascadeSplits() const;

 private:
  glm::vec3 m_LastLightDirection;  ///< 上一次计算时的光源方向
  glm::mat4 m_LastCameraView;      ///< 上一次计算时的相机视图矩阵
  glm::mat4 m_LastCameraProj;      ///< 上一次计算时的相机投影矩阵

  /**
   * @brief 计算级联分割距离
   * @param cameraProj 相机投影矩阵
   * @note 使用对数分割策略平衡近处和远处的阴影质量
   */
  void CalculateCascadeSplits(const glm::mat4 &cameraProj);

  /**
   * @brief 计算级联的阴影矩阵
   * @param lightDirection 光源方向
   * @param cameraView 相机视图矩阵
   * @param cameraProj 相机投影矩阵
   * @note 为每个级联计算正交投影的阴影矩阵
   */
  void CalculateCascadeMatrices(const glm::vec3 &lightDirection,
                                const glm::mat4 &cameraView,
                                const glm::mat4 &cameraProj);

  /**
   * @brief 计算特定级联的视锥体角点
   * @param nearPlane 级联近平面
   * @param farPlane 级联远平面
   * @param cameraView 相机视图矩阵
   * @param cameraProj 相机投影矩阵
   * @return 视锥体8个角点的世界坐标
   */
  std::array<glm::vec3, 8> CalculateFrustumCorners(float nearPlane,
                                                   float farPlane,
                                                   const glm::mat4 &cameraView,
                                                   const glm::mat4 &cameraProj) const;

  /**
   * @brief 检查是否需要更新阴影数据
   * @param newLightDirection 新的光源方向
   * @param newCameraView 新的相机视图矩阵
   * @param newCameraProj 新的相机投影矩阵
   * @return 是否发生变换（超过阈值）
   */
  bool HasTransformChanged(const glm::vec3 &newLightDirection,
                           const glm::mat4 &newCameraView,
                           const glm::mat4 &newCameraProj) const;
};

}  // namespace mite

#endif  // MITE_DIRECTIONAL_SHADOW_MAP_H
