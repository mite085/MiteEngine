#ifndef MITE_UNIFORM_BUFFER
#define MITE_UNIFORM_BUFFER

#include "headers/headers.h"

namespace mite {
/**
 * @brief 相机参数UBO结构体
 * @note 按照std140布局规则对齐，包含所有材质参数
 */
struct alignas(16) CameraUBO {
  // ---- 矩阵部分 (3 * 64 = 192字节) ----
  glm::mat4 view;            // 64字节 - 视图矩阵
  glm::mat4 projection;      // 64字节 - 投影矩阵
  glm::mat4 viewProjection;  // 64字节 - 视图投影矩阵

  // ---- 相机参数部分 (2 * 12 + 2 * 4 + 192 = 224字节) ----
  glm::vec3 position;  // 12字节 - 相机世界坐标（vec3占用12字节，但整个块是16字节）
  float nearPlane;     // 4字节  - 近平面距离  （后面的标量可以占用剩余的4字节）
  glm::vec3 forward;   // 12字节 - 相机前向向量（但下个vec3必须从新的16字节开始）
  float farPlane;      // 4字节  - 远平面距离  （如果没有跟随float/int，则vec3应当占16字节）

  // ---- 投影参数部分 (3 * 4 + 224 = 236字节) ----
  float fov;			// 4字节 - 垂直FOV（弧度）
  float orthoSize;		// 4字节 - 正交投影尺寸
  int projectionType;	// 4字节 - 投影类型标志 (1 = 透视, 0 = 正交)

  // 4字节填充
  float padding;  

  // 总大小: 236 + 4 = 240字节 (16字节对齐)
};
/**
 * @brief GBuffer材质参数UBO结构体
 * @note 按照std140布局规则对齐，包含所有材质参数
 */
struct alignas(16) GBufferMaterialUBO {
  // ---- 基础PBR参数 (4 * 16 = 64 字节) ----
  glm::vec4 baseColor;            // RGB + Alpha (w分量)
  glm::vec4 metallicRoughnessAO;  // x: metallic, y: roughness, z: AO, w: unused
  glm::vec4 emission;             // RGB + Intensity (w分量)
  glm::vec4 normalScale;          // x: normal scale, yzw: unused

  // ---- NPR参数 (2 * 16 + 64 = 96 字节) ----
  glm::vec4 nprParameters;  // xyzw：rampThreshold, rampSmoothness, specularSize, outlineWidth
  glm::vec4 nprColors;      // xyz: shadowTint, w: rimPower

  // ---- 纹理标识和参数 (7 * 16 + 96 = 208 字节) ----
  glm::vec4 textureCNMROFlags;    // xyzw has: BaseColorTex, NormalTex, MRTex, OcclusionTex
  glm::vec4 textureEmissionFlag;  // x: hasEmissiveTex, yzw: reserved
  glm::vec4 baseColorTexParams;   // xy: scale, zw: offset
  glm::vec4 normalTexParams;      // xy: scale, zw: offset
  glm::vec4 mrTexParams;          // xy: scale, zw: offset
  glm::vec4 emissiveTexParams;    // xy: scale, zw: offset
  glm::vec4 occlusionTexParams;   // xy: scale, zw: offset

  // ---- 渲染属性 (1 * 16 + 192 = 224 字节) ----
  glm::vec4 renderProperties;  // x: alphaCutoff, y: doubleSided, z: alphaMode, w: unused

  // 总大小: 224 字节 (16字节对齐)
};
};  // namespace mite

#endif
