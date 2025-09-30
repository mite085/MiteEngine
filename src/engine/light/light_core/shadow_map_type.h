#ifndef MITE_SHADOW_CONFIG_H
#define MITE_SHADOW_CONFIG_H

#include "headers/headers.h"

namespace mite {
/**
 * @brief 阴影质量配置
 */
enum class ShadowQuality {
  LOW = 0,     // 512x512
  MEDIUM = 1,  // 1024x1024
  HIGH = 2,    // 2048x2048
  ULTRA = 3    // 4096x4096
};
/**
 * @brief 阴影过滤模式
 */
enum class ShadowFilter {
  NONE = 0,  // 无过滤
  PCF = 1,   // 百分比渐进过滤
  VSM = 2    // 方差阴影映射
};
/**
 * @brief 统一的阴影数据容器
 * @note 使用union管理不同类型光源的阴影数据
 */
struct ShadowMapData {
  // 基础阴影配置
  bool enabled = false;
  ShadowQuality quality = ShadowQuality::MEDIUM;
  ShadowFilter filter = ShadowFilter::PCF;
  float bias = 0.005f;
  float normalBias = 0.01f;

  // 阴影贴图索引（由Renderer分配）
  int shadowMapIndex = -1;

  // 类型特定的阴影数据 - 使用union节省空间
  union {
    // 点光源阴影数据（立方体贴图）
    struct {
      float nearPlane = 0.1f;
      float farPlane = 25.0f;
      std::array<glm::mat4, 6> faceViewProjMatrices;  // 立方体六个面
    } point;

    // 聚光灯阴影数据
    struct {
      float nearPlane = 0.1f;
      float farPlane = 25.0f;
      glm::mat4 viewMatrix;
      glm::mat4 projectionMatrix;
    } spot;

    // 方向光阴影数据（级联阴影）
    struct {
      int cascadeCount = 4;
      float splitLambda = 0.95f;
      std::array<float, 5> cascadeSplits;        // 级联分割距离
      std::array<glm::mat4, 4> cascadeMatrices;  // 级联VP矩阵
    } directional;

    // 面光源阴影数据
    struct {
      float nearPlane = 0.1f;
      float farPlane = 10.0f;
      glm::mat4 viewMatrix;
      glm::mat4 projectionMatrix;
    } area;
  } specific;

  // 有效性标志
  bool isValid = false;

  /**
   * @brief 默认构造函数
   */
  ShadowMapData() : specific{} {}

  /**
   * @brief 获取阴影贴图尺寸
   */
  uint32_t GetShadowMapSize() const
  {
    switch (quality) {
      case ShadowQuality::LOW:
        return 512;
      case ShadowQuality::MEDIUM:
        return 1024;
      case ShadowQuality::HIGH:
        return 2048;
      case ShadowQuality::ULTRA:
        return 4096;
      default:
        return 1024;
    }
  }

  /**
   * @brief 获取阴影矩阵数量
   */
  size_t GetShadowMatrixCount() const
  {
    if (!enabled)
      return 0;

    // 根据配置推断光源类型
    if (specific.directional.cascadeCount > 0) {
      return specific.directional.cascadeCount;  // 方向光使用级联
    }
    else if (specific.point.farPlane > 0) {
      return 6;  // 点光源使用立方体贴图
    }
    else {
      return 1;  // 聚光灯和面光源使用单个矩阵
    }
  }

  /**
   * @brief 获取特定索引的阴影矩阵
   */
  glm::mat4 GetShadowMatrix(size_t index) const
  {
    if (!enabled || !isValid) {
      return glm::mat4(1.0f);
    }

    // 根据配置推断光源类型
    if (specific.directional.cascadeCount > 0 && index < 4) {
      return specific.directional.cascadeMatrices[index];  // 方向光
    }
    else if (specific.point.farPlane > 0 && index < 6) {
      return specific.point.faceViewProjMatrices[index];  // 点光源
    }
    else if (specific.spot.farPlane > 0 && index == 0) {
      return specific.spot.projectionMatrix * specific.spot.viewMatrix;  // 聚光灯
    }
    else if (specific.area.farPlane > 0 && index == 0) {
      return specific.area.projectionMatrix * specific.area.viewMatrix;  // 面光源
    }

    LOG_ERROR("Invalid shadow matrix index: {}", index);
    return glm::mat4(1.0f);
  }
};

}  // namespace mite

#endif  // MITE_SHADOW_CONFIG_H
