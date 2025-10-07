#ifndef MITE_UNIFORM_BUFFER
#define MITE_UNIFORM_BUFFER

#include "headers/headers.h"

namespace mite {
// ---- 资源类型枚举 ----
enum class ShaderBufferResourceType {
  // UBO类型 (绑定点范围: 0-15)
  CameraUBO = 0,  // 相机参数 - 绑定点 0
  MaterialUBO,    // 材质参数 - 绑定点 1
  ModelUBO,       // 模型矩阵 - 绑定点 2
  SceneUBO,       // 场景全局参数 - 绑定点 3（暂未启用）

  // SSBO类型 (绑定点范围: 16-31)
  LightSSBO = 16,  // 光源数据 - 绑定点 16
  InstanceSSBO,    // 实例数据 - 绑定点 17（暂未启用）
  BoneSSBO,        // 骨骼动画数据 - 绑定点 18（暂未启用）

  // 纹理类型 (绑定点范围: 32-95)
  //
  // PBR材质纹理 (32-39)
  BaseColorTexture = 32,     // 基础色纹理 - 绑定点 32
  NormalTexture,             // 法线纹理 - 绑定点 33
  MetallicRoughnessTexture,  // 金属粗糙度纹理 - 绑定点 34
  EmissiveTexture,           // 自发光纹理 - 绑定点 35
  OcclusionTexture,          // 环境光遮蔽纹理 - 绑定点 36
  // 阴影和环境纹理 (40-47)
  ShadowMap = 40,  // 阴影贴图 - 绑定点 40
  EnvironmentMap,  // 环境贴图 - 绑定点 41
  BRDFLUT,         // BRDF查找表 - 绑定点 42（暂未启用）
  IrradianceMap,   // 辐照度图 - 绑定点 43（暂未启用）
  PrefilterMap,    // 预滤波环境图 - 绑定点 44（暂未启用）
  // 后期处理纹理 (48-55)
  ColorGradingLUT = 48,  // 色彩分级LUT - 绑定点 48（暂未启用）
  BloomTexture,          // 泛光纹理 - 绑定点 49（暂未启用）
  SSAOTexture,           // SSAO纹理 - 绑定点 50（暂未启用）
  // 自定义纹理 (56-95)
  CustomTexture0 = 56,  // 自定义纹理0 - 绑定点 56（暂未启用）

  // 尾部计数用无效类型
  Count = 96,
};

// ---- 常用资源名称定义 ----
struct ShaderBufferResourceNames {
  // UBO名称
  static constexpr const char *CAMERA_UBO = "CameraUBO";
  static constexpr const char *MATERIAL_UBO = "MaterialUBO";
  static constexpr const char *MODEL_UBO = "ModelUBO";
  static constexpr const char *SCENE_UBO = "SceneUBO";

  // SSBO名称
  static constexpr const char *LIGHT_SSBO = "LightsSSBO";
  static constexpr const char *INSTANCE_SSBO = "InstanceSSBO";
  static constexpr const char *BONE_SSBO = "BoneSSBO";

  // 纹理名称
  static constexpr const char *BASE_COLOR_TEXTURE = "u_BaseColorTexture";
  static constexpr const char *NORMAL_TEXTURE = "u_NormalTexture";
  static constexpr const char *METALLIC_ROUGHNESS_TEXTURE = "u_MetallicRoughnessTexture";
  static constexpr const char *EMISSIVE_TEXTURE = "u_EmissiveTexture";
  static constexpr const char *OCCLUSION_TEXTURE = "u_OcclusionTexture";

  static constexpr const char *SHADOW_MAP = "u_ShadowMap";
  static constexpr const char *ENVIRONMENT_MAP = "u_EnvironmentMap";
  static constexpr const char *BRDF_LUT = "u_BRDFLUT";
  static constexpr const char *IRRADIANCE_MAP = "u_IrradianceMap";
  static constexpr const char *PREFILTER_MAP = "u_PrefilterMap";

  static constexpr const char *COLOR_GRADING_LUT = "u_ColorGradingLUT";
  static constexpr const char *BLOOM_TEXTURE = "u_BloomTexture";
  static constexpr const char *SSAO_TEXTURE = "u_SSAOTexture";
};

// ------------------------ 材质参数标准定义 ------------------------
// 材质标准参数键名（与着色器Uniform名称对应，用于Asset模块MaterialMetadata通用参数检索）
namespace MaterialParamKeys {
// 基础PBR参数
static constexpr const char *BASE_COLOR = "u_BaseColor";                  // vec4 (RGBA)
static constexpr const char *METALLIC = "u_Metallic";                     // float
static constexpr const char *ROUGHNESS = "u_Roughness";                   // float
static constexpr const char *AO = "u_AO";                                 // float
static constexpr const char *EMISSION_COLOR = "u_EmissionColor";          // vec3
static constexpr const char *EMISSION_INTENSITY = "u_EmissionIntensity";  // float
static constexpr const char *NORMAL_SCALE = "u_NormalScale";              // float

// 纹理标识
static constexpr const char *HAS_BASE_COLOR_TEX = "u_HasBaseColorTexture";  // float
static constexpr const char *HAS_NORMAL_TEX = "u_HasNormalTexture";         // float
static constexpr const char *HAS_MR_TEX = "u_HasMetallicRoughnessTexture";  // float
static constexpr const char *HAS_EMISSIVE_TEX = "u_HasEmissiveTexture";     // float
static constexpr const char *HAS_OCCLUSION_TEX = "u_HasOcclusionTexture";   // float

// 纹理槽位名称（与着色器纹理采样器名称对应）
static constexpr const char *BASE_COLOR_TEXTURE = ShaderBufferResourceNames::BASE_COLOR_TEXTURE;
static constexpr const char *NORMAL_TEXTURE = ShaderBufferResourceNames::NORMAL_TEXTURE;
static constexpr const char *METALLIC_ROUGHNESS_TEXTURE = ShaderBufferResourceNames::METALLIC_ROUGHNESS_TEXTURE;
static constexpr const char *EMISSIVE_TEXTURE = ShaderBufferResourceNames::EMISSIVE_TEXTURE;
static constexpr const char *OCCLUSION_TEXTURE = ShaderBufferResourceNames::OCCLUSION_TEXTURE;
}  // namespace MaterialParamKeys

// ---- 纹理绑定点辅助结构 ----
struct TextureBindingPoints {
  // PBR材质纹理
  static constexpr uint32_t BASE_COLOR = 32;
  static constexpr uint32_t NORMAL = 33;
  static constexpr uint32_t METALLIC_ROUGHNESS = 34;
  static constexpr uint32_t EMISSIVE = 35;
  static constexpr uint32_t OCCLUSION = 36;

  // 阴影和环境
  static constexpr uint32_t SHADOW_MAP = 40;
  static constexpr uint32_t ENVIRONMENT_MAP = 41;
  static constexpr uint32_t BRDF_LUT = 42;
  static constexpr uint32_t IRRADIANCE_MAP = 43;
  static constexpr uint32_t PREFILTER_MAP = 44;

  // 后期处理
  static constexpr uint32_t COLOR_GRADING_LUT = 48;
  static constexpr uint32_t BLOOM_TEXTURE = 49;
  static constexpr uint32_t SSAO_TEXTURE = 50;
};

/**
 * @brief 相机参数UBO结构体
 * @note 按照std140布局规则对齐，包含所有材质参数
 */
struct alignas(16) CameraUniformBuffer {
  // ---- 矩阵部分 (3 * 64 = 192字节) ----
  glm::mat4 view;            // 64字节 - 视图矩阵
  glm::mat4 projection;      // 64字节 - 投影矩阵
  glm::mat4 viewProjection;  // 64字节 - 视图投影矩阵

  // ---- 相机参数部分 (2 * 12 + 2 * 4 + 192 = 224字节) ----
  glm::vec3 position;  // 12字节 - 相机世界坐标（vec3占用12字节，但整个块是16字节）
  float nearPlane;  // 4字节  - 近平面距离  （后面的标量可以占用剩余的4字节）
  glm::vec3 forward;  // 12字节 - 相机前向向量（但下个vec3必须从新的16字节开始）
  float farPlane;  // 4字节  - 远平面距离  （如果没有跟随float/int，则vec3应当占16字节）

  // ---- 投影参数部分 (3 * 4 + 224 = 236字节) ----
  float fov;           // 4字节 - 垂直FOV（弧度）
  float orthoSize;     // 4字节 - 正交投影尺寸
  int projectionType;  // 4字节 - 投影类型标志 (1 = 透视, 0 = 正交)

  // 4字节填充
  float padding;

  // 总大小: 236 + 4 = 240字节 (16字节对齐)
};
/**
 * @brief GBuffer材质参数UBO结构体
 * @note 按照std140布局规则对齐，包含所有材质参数
 */
struct alignas(16) MaterialUniformBuffer {
  // ---- 材质类型标识（1 * 16 = 16 字节） ----
  glm::vec4 materialInfo;  // x: MaterialType, yzw: 保留

  // ---- 基础PBR参数 (4 * 16 + 16 = 80 字节) ----
  glm::vec4 baseColor;            // RGB + Alpha (w分量)
  glm::vec4 metallicRoughnessAO;  // x: metallic, y: roughness, z: AO, w: unused
  glm::vec4 emission;             // RGB + Intensity (w分量)
  glm::vec4 normalScale;          // x: normal scale, yzw: unused

  // ---- NPR参数 (2 * 16 + 80 = 112 字节) ----
  glm::vec4 nprParameters;  // xyzw：rampThreshold, rampSmoothness, specularSize, outlineWidth
  glm::vec4 nprColors;      // xyz: shadowTint, w: rimPower

  // ---- 纹理标识和参数 (7 * 16 + 96 = 224 字节) ----
  glm::vec4 textureCNMROFlags;    // xyzw has: BaseColorTex, NormalTex, MRTex, OcclusionTex
  glm::vec4 textureEmissionFlag;  // x: hasEmissiveTex, yzw: reserved
  glm::vec4 baseColorTexParams;   // xy: scale, zw: offset
  glm::vec4 normalTexParams;      // xy: scale, zw: offset
  glm::vec4 mrTexParams;          // xy: scale, zw: offset
  glm::vec4 emissiveTexParams;    // xy: scale, zw: offset
  glm::vec4 occlusionTexParams;   // xy: scale, zw: offset

  // ---- 渲染属性 (1 * 16 + 224 = 240 字节) ----
  glm::vec4 renderProperties;  // x: alphaCutoff, y: doubleSided, z: alphaMode, w: unused

  // 总大小: 240 字节 (16字节对齐)
};

/**
 * @brief 模型矩阵UBO结构体
 */
struct alignas(16) ModelUniformBuffer {
  glm::mat4 model;
  glm::mat4 normalMatrix;  // 用于法线变换的矩阵
};
};  // namespace mite

#endif
