// 全局Uniform缓冲区定义
#ifndef UNIFORMS_GLSL
#define UNIFORMS_GLSL

#include "common.glsl"

// 相机UBO
layout(std140, binding = 0) uniform CameraUBO {
    mat4 view;              // 视图矩阵
    mat4 projection;        // 投影矩阵
    mat4 viewProjection;    // 视图投影矩阵
    vec3 position;          // 相机世界坐标
    float nearPlane;        // 近平面
    float farPlane;         // 远平面
    vec2 padding0;
} u_Camera;

// 材质UBO - 与C++端GBufferMaterialUBO结构体对应
layout(std140, binding = 1) uniform MaterialUBO {
    // 基础PBR参数
    vec4 baseColor;
    vec4 metallicRoughnessAO;  // x: metallic, y: roughness, z: AO
    vec4 emission;             // RGB + Intensity (w分量)
    vec4 normalScale;          // x: normal scale
    
    // NPR参数
    vec4 nprParameters;  // xyzw: rampThreshold, rampSmoothness, specularSize, outlineWidth
    vec4 nprColors;      // xyz: shadowTint, w: rimPower
    
    // 纹理标识
    vec4 textureCNMROFlags;    // xyzw: hasBaseColorTex, hasNormalTex, hasMRTex, hasOcclusionTex
    vec4 textureEmissionFlag;  // x: hasEmissiveTex
    
    // 纹理参数
    vec4 baseColorTexParams;   // xy: scale, zw: offset
    vec4 normalTexParams;
    vec4 mrTexParams;
    vec4 emissiveTexParams;
    vec4 occlusionTexParams;
    
    // 渲染属性
    vec4 renderProperties;  // x: alphaCutoff, y: doubleSided, z: alphaMode
} u_Material;

#endif // UNIFORMS_GLSL
