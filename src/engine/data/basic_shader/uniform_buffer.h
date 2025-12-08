#ifndef MITE_UNIFORM_BUFFER
#define MITE_UNIFORM_BUFFER

#include "basic_type/light_type.h"
#include "basic_type/texture_type.h"

namespace mite {
// ---- 常用UBO/SSBO/Texture名称定义 ----
struct ShaderBufferResourceNames {
  // UBO名称
  static constexpr const char *CAMERA_UBO = "CameraUBO";
  static constexpr const char *MATERIAL_UBO = "MaterialUBO";
  static constexpr const char *MODEL_UBO = "ModelUBO";
  static constexpr const char *SHADOW_UBO = "ShadowUBO";
  static constexpr const char *SHADOW_RENDER_CONTEXT_UBO = "ShadowRenderContextUBO";
  static constexpr const char *SCENE_UBO = "SceneUBO";

  // SSBO名称
  static constexpr const char *LIGHT_SSBO = "LightsSSBO";
  static constexpr const char *INSTANCE_SSBO = "InstanceSSBO";
  static constexpr const char *BONE_SSBO = "BoneSSBO";

  // 运行时纹理名称（注释部分暂未启用）
  static constexpr const char *GBUFFER_WORLD_POS_DEPTH = "u_GBufferWorldPosDepth";
  static constexpr const char *GBUFFER_BASE_COLOR_MAT_TYPE = "u_GBufferBaseColorMatType";
  static constexpr const char *GBUFFER_METALLIC_ROUGHNESS_AO = "u_GBufferMetallicRoughnessAO";
  static constexpr const char *GBUFFER_NORMAL_SCALE = "u_GBufferNormalScale";
  static constexpr const char *GBUFFER_EMISSION_ALPHA = "u_GBufferEmissionAlpha";
  static constexpr const char *GBUFFER_NPR_PARAM = "u_GBufferNPRParam";
  static constexpr const char *GBUFFER_NPR_COLOR = "u_GBufferNPRColor";
  static constexpr const char *SHADOW_MAP_DIRECTIONAL = "u_ShadowMapDirectional";
  static constexpr const char *SHADOW_MAP_POINT = "u_ShadowMapPoint";
  static constexpr const char *SHADOW_MAP_SPOT = "u_ShadowMapSpot";
  static constexpr const char *SHADOW_MAP_AREA = "u_ShadowMapArea";
  static constexpr const char *LIGHTING_DIFFUSE = "u_LightingDiffuse";
  static constexpr const char *LIGHTING_SPECULAR = "u_LightingSpecular";
  static constexpr const char *LIGHTING_COMBINED = "u_LightingCombined";
  static constexpr const char *LIGHTING_AMBIENT = "u_LightingAmbient";
  static constexpr const char *FORWARD_TRANSPARENT = "u_ForwardTransparent";
  static constexpr const char *FORWARD_BLEND = "u_ForwardBlend";
  // static constexpr const char *POSTPROCESS_BLOOM = "u_PostProcessBloom";
  // static constexpr const char *POSTPROCESS_TONE_MAPPED = "u_PostProcessToneMapped";
  // static constexpr const char *POSTPROCESS_FINAL = "u_PostProcessFinal";
  static constexpr const char *RENDER_TARGET = "u_RenderTarget";
  static constexpr const char *DEPTH_TEXTURE = "u_DepthTexture";
  static constexpr const char *STENCIL_TEXTURE = "u_StencilTexture";
  // static constexpr const char *DEBUG_VIEW = "u_DebugView";
  // static constexpr const char *UI_OVERLAY = "u_UIOverlay";

  // 外部加载纹理名称（注释部分暂未启用）
  static constexpr const char *BASE_COLOR_TEXTURE = "u_BaseColorTexture";
  static constexpr const char *NORMAL_TEXTURE = "u_NormalTexture";
  static constexpr const char *METALLIC_ROUGHNESS_TEXTURE = "u_MetallicRoughnessTexture";
  static constexpr const char *EMISSIVE_TEXTURE = "u_EmissiveTexture";
  static constexpr const char *OCCLUSION_TEXTURE = "u_OcclusionTexture";
  static constexpr const char *ENVIRONMENT_MAP = "u_EnvironmentMap";
  // static constexpr const char *BRDF_LUT = "u_BRDFLUT";
  // static constexpr const char *IRRADIANCE_MAP = "u_IrradianceMap";
  // static constexpr const char *PREFILTER_MAP = "u_PrefilterMap";
  // static constexpr const char *COLOR_GRADING_LUT = "u_ColorGradingLUT";
  // static constexpr const char *BLOOM_TEXTURE = "u_BloomTexture";
  // static constexpr const char *SSAO_TEXTURE = "u_SSAOTexture";
  // static constexpr const char *CUSTOM_TEXTURE_0 = "u_CustomTexture0";
  // static constexpr const char *CUSTOM_TEXTURE_1 = "u_CustomTexture1";
  // static constexpr const char *CUSTOM_TEXTURE_2 = "u_CustomTexture2";
  // static constexpr const char *CUSTOM_TEXTURE_3 = "u_CustomTexture3";
};

// ------------------------ 外部载入材质参数/纹理参数定义 ------------------------
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

// 纹理槽位名称（与着色器纹理资源名称对应）
static constexpr const char *BASE_COLOR_TEXTURE = ShaderBufferResourceNames::BASE_COLOR_TEXTURE;
static constexpr const char *NORMAL_TEXTURE = ShaderBufferResourceNames::NORMAL_TEXTURE;
static constexpr const char *METALLIC_ROUGHNESS_TEXTURE =
    ShaderBufferResourceNames::METALLIC_ROUGHNESS_TEXTURE;
static constexpr const char *EMISSIVE_TEXTURE = ShaderBufferResourceNames::EMISSIVE_TEXTURE;
static constexpr const char *OCCLUSION_TEXTURE = ShaderBufferResourceNames::OCCLUSION_TEXTURE;
}  // namespace MaterialParamKeys

// ---- 资源类型枚举 ----
/**
 * @brief UBO 资源类型枚举
 * @note 使用独立的 UBO 绑定点命名空间
 */
enum class UBOResourceType {
  CameraUBO = 0,           // 相机参数 binding = 0
  MaterialUBO,             // 材质参数 binding = 1
  ModelUBO,                // 模型矩阵 binding = 2
  ShadowUBO,               // 阴影 binding = 3
  ShadowRenderContextUBO,  // 阴影渲染上下文 binding = 4
  SceneUBO,                // 场景全局参数(未启用)
  Count                    // 类型计数
};
/**
 * @brief SSBO 资源类型枚举
 * @note 使用独立的 SSBO 绑定点命名空间
 */
enum class SSBOResourceType {
  LightSSBO = 0,  // 光源数据
  InstanceSSBO,   // 实例数据
  BoneSSBO,       // 骨骼动画数据
  Count           // 类型计数
};
/**
 * @brief 纹理资源类型枚举
 * @note 使用独立的纹理单元命名空间
 */
enum class TextureResourceType {
  // 运行时纹理类型
  RuntimeTexture = 0,
  // 外部加载纹理类型
  ExternalTexture,
  Count
};
// ---- 绑定点范围定义（基于实际硬件查询） ----
struct BindingRanges {
  // 查询硬件实际限制
  static uint32_t GetMaxTextureUnits()
  {
    static GLint maxUnits = []() {
      GLint units;
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);
      return units > 0 ? units : 32;  // 默认值
    }();
    return static_cast<uint32_t>(maxUnits);
  }

  static uint32_t GetMaxUBOBindings()
  {
    static GLint maxBindings = []() {
      GLint bindings;
      glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &bindings);
      return bindings > 0 ? bindings : 72;  // 默认值
    }();
    return static_cast<uint32_t>(maxBindings);
  }

  static uint32_t GetMaxSSBOBindings()
  {
    static GLint maxBindings = []() {
      GLint bindings;
      glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &bindings);
      return bindings > 0 ? bindings : 8;  // 默认值
    }();
    return static_cast<uint32_t>(maxBindings);
  }
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
  float nearPlane;     // 4字节  - 近平面距离  （后面的标量可以占用剩余的4字节）
  glm::vec3 forward;   // 12字节 - 相机前向向量（但下个vec3必须从新的16字节开始）
  float farPlane;      // 4字节  - 远平面距离  （如果没有跟随float/int，则vec3应当占16字节）

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

/**
 * @brief Shadow相关UBO结构体，用于ShadowMap生成
 */
struct alignas(16) ShadowUniformBuffer {
  // ---- 方向光源阴影参数 ----
  // 级联分割距离 (MAX_CASCADES(4) * 16 = 64字节)
  glm::vec4 cascadeSplits[MAX_CASCADES];  // 每个级联的远平面距离

  // 方向光源阴影矩阵 (MAX_DIRECTIONAL_LIGHTS(8) * MAX_CASCADES(4) * 64 = 2048字节)
  glm::mat4 directionalMatrices[MAX_DIRECTIONAL_LIGHTS * MAX_CASCADES];

  // ---- 点光源阴影参数 ----
  // 点光源阴影矩阵 (MAX_POINT_LIGHTS(16) * 6 * 64 = 6144字节)
  glm::mat4 pointLightMatrices[MAX_POINT_LIGHTS * 6];  // 每个点光源6个面

  // ---- 聚光灯阴影参数 ----
  // 聚光灯阴影矩阵 (MAX_SPOT_LIGHTS(32) * 64 = 2048字节)
  glm::mat4 spotLightMatrices[MAX_SPOT_LIGHTS];

  // ---- 光源计数和配置 ----
  // x: directionalCount, y: pointLightCount, z: spotLightCount, w: cascadeCount (16字节)
  glm::ivec4 shadowConfig = glm::ivec4(0, 0, 0, 0);

  // ---- 通用阴影参数 ----
  // x: shadowBias, y: normalBias, z: shadowFilterSize, w: shadowMapSize (16字节)
  glm::vec4 shadowParams = glm::vec4(0.001f, 0.02f, 1.0f, 1024.0f);

  // ---- 光源特定阴影索引 ----
  // 方向光源阴影索引 (MAX_DIRECTIONAL_LIGHTS(8) * 16 = 128字节)
  glm::ivec4 directionalShadowIndices[MAX_DIRECTIONAL_LIGHTS];  // 每个光源的阴影图起始索引

  // 点光源阴影索引 (MAX_POINT_LIGHTS(16) * 16 = 256字节)
  glm::ivec4 pointShadowIndices[MAX_POINT_LIGHTS];  // 每个点光源在立方体贴图数组中的索引

  // 聚光灯阴影索引 (MAX_SPOT_LIGHTS(32) * 16 = 512字节)
  glm::ivec4 spotShadowIndices[MAX_SPOT_LIGHTS];  // 每个聚光灯在阴影图数组中的索引

  // 面光源阴影索引 (MAX_AREA_LIGHTS(8) * 16 = 128字节)（未启用）
  glm::ivec4 areaShadowIndices[MAX_AREA_LIGHTS];  // 每个面光源在阴影图数组中的索引（未启用）

  // 总大小: 64 + 2048 + 6144 + 2048 + 16 + 16 + 1024 = 11360字节
  // 11360字节 < 64KB(65536字节，OpenGL最低标准)，符合UBO大小限制
};

/**
 * @brief 阴影渲染上下文UBO结构体
 * @note 用于ShadowMap生成时的渲染上下文传递
 */
struct alignas(16) ShadowRenderContextUniformBuffer {
  // x: lightIndex, y: cascadeIndex, z: faceIndex, w: shadowMapType
  glm::ivec4 shadowRenderContext;
  // x: currentDepth, y: shadowMapSize, z: lightRange, w: padding(预留参数，暂未启用)
  glm::vec4 shadowRenderParams;
};
};  // namespace mite

#endif
