#ifndef MITE_DIRECTIONAL_SHADOW_MAP_H
#define MITE_DIRECTIONAL_SHADOW_MAP_H

#include "light_core/shadow_map.h"

namespace mite {
/**
 * @brief 方向光阴影贴图类
 * @note 方向光使用级联阴影技术，需要多级ShadowMap纹理
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
   * @param cameraView 相机视图矩阵（点光源不使用）
   * @param cameraProj 相机投影矩阵（点光源不使用）
   * @return 更新后的阴影数据
   * @note 点光源阴影基于光源位置计算六个面的立方体贴图矩阵
   */
  ShadowMapData PrepareShadowData(const Transform &lightWorldTransform,
                                  const Transform &cameraView,
                                  const glm::mat4 &cameraProj = glm::mat4(1.0f)) override;

  /**
   * @brief 获取阴影矩阵数量
   * @return 固定返回6，对应立方体贴图的六个面
   */
  size_t GetShadowMatrixCount() const override;

  /**
   * @brief 获取特定索引的阴影矩阵
   * @param index 矩阵索引（0-5对应立方体六个面）
   * @return 对应的阴影视图投影矩阵
   * @note 索引顺序：+X, -X, +Y, -Y, +Z, -Z
   */
  glm::mat4 GetShadowMatrix(size_t index) const override;

  /**
   * @brief 检查是否需要更新阴影数据
   * @return 是否需要更新
   * @note 当光源移动或配置改变时需要更新
   */
  bool NeedsUpdate() const override;

  /**
   * @brief 标记阴影数据已更新
   */
  void MarkUpdated() override;

  /**
   * @brief 获取阴影类型名称
   * @return 类型名称字符串 "PointShadowMap"
   */
  std::string GetShadowTypeName() const override;
};
}  // namespace mite

#endif  // MITE_POINT_SHADOW_MAP_H
