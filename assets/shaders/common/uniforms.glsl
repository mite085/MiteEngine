// 全局Uniform缓冲区定义
#ifndef UNIFORMS_GLSL
#define UNIFORMS_GLSL

#include "common.glsl"

// 相机UBO - 与C++端CameraUBO结构体对应
layout(std140, binding = 0) uniform CameraBuffer {
    mat4 view;              // 视图矩阵
    mat4 projection;        // 投影矩阵  
    mat4 viewProjection;    // 视图投影矩阵
    
    vec3 cameraPosition;    // 相机世界坐标
    float nearPlane;        // 近平面距离
    
    vec3 cameraForward;     // 相机前向向量
    float farPlane;         // 远平面距离
    
    float fov;              // 垂直FOV（弧度）
    float orthoSize;        // 正交投影尺寸
    int projectionType;     // 投影类型标志
    float padding;          // 填充
} u_Camera;

// 材质UBO - 与C++端GBufferMaterialUBO结构体对应
layout(std140, binding = 1) uniform MaterialBuffer {
    // 基础PBR参数
    vec4 baseColor;             // RGB + Alpha
    vec4 metallicRoughnessAO;   // x:金属度, y:粗糙度, z:AO, w:未使用
    vec4 emission;              // RGB + 强度(w分量)
    vec4 normalScale;           // x:法线贴图强度, yzw:未使用
    
    // NPR参数
    vec4 nprParameters;         // x:色阶阈值, y:色阶平滑度, z:高光尺寸, w:描边宽度
    vec4 nprColors;             // xyz:阴影色调, w:边缘光强度
    
    // 纹理标识
    vec4 textureCNMROFlags;     // xyzw: 基础色/法线/金属粗糙度/遮挡纹理标志
    vec4 textureEmissionFlag;   // x:自发光纹理标志, yzw:保留
    
    // 纹理参数
    vec4 baseColorTexParams;    // xy:缩放, zw:偏移
    vec4 normalTexParams;       // xy:缩放, zw:偏移  
    vec4 mrTexParams;          // xy:缩放, zw:偏移
    vec4 emissiveTexParams;    // xy:缩放, zw:偏移
    vec4 occlusionTexParams;   // xy:缩放, zw:偏移
    
    // 渲染属性
    vec4 renderProperties;      // x:透明度阈值, y:双面渲染, z:Alpha模式, w:未使用
} u_Material;

#endif
