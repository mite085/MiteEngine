#ifndef MITE_SHADOW_CONFIG_H
#define MITE_SHADOW_CONFIG_H

#include "headers/headers.h"

namespace mite {
/**
 * @brief 阴影质量配置
 */
enum class ShadowQuality : uint32_t {
  LOW = 512,      // 512x512
  MEDIUM = 1024,  // 1024x1024
  HIGH = 2048,    // 2048x2048
  ULTRA = 4096    // 4096x4096
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
      unsigned int cascadeCount = 4;
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
};
}  // namespace mite

#endif  // MITE_SHADOW_CONFIG_H
