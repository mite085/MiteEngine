// 光源SSBO定义 - 与C++端LightSSBO数据结构对应
#ifndef LIGHTS_SSBO_GLSL
#define LIGHTS_SSBO_GLSL

#include "common.glsl"

// SSBO头部信息 - 与LightSSBOHeader对应
struct LightSSBOHeader {
    int lightCount;         // 有效光源数量
    float padding[3];       // 填充确保16字节对齐
};

// GPU光源数据 - 与GPULightData对应
struct GPULightData {
    // 基础属性 (32字节)
    vec3 color;             // 光源颜色
    float intensity;        // 光源强度
    vec3 position;          // 世界坐标位置
    float type;             // 光源类型 (LightType转换为float)
    vec3 direction;         // 光源方向/法线
    float padding1;         // 填充
    
    // 类型特定属性 - 使用数组避免union
    float specificData[8];  // 统一存储所有类型特定数据
};

// 光源SSBO绑定 - 占用绑定点16 (参考ShaderBufferBindingRanges设定)
layout(std430, binding = 0) buffer LightsSSBO {
    LightSSBOHeader header;         // 头部信息
    GPULightData lights[MAX_LIGHTS]; // 光源数据数组
} u_Lights;

// 光源特定数据访问辅助函数--点光源/聚光灯专用
float GetLightRange(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[0];
}

float GetLightInnerAngle(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[1];
}

float GetLightOuterAngle(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[2];
}

float GetLightBlend(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[3];
}

float GetLightFalloff(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[4];
}

// 光源特定数据访问辅助函数--方向光专用
float GetLightIrradiance(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[0];
}

// 光源特定数据访问辅助函数--面光源专用
float GetAreaLightPower(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[0];
}

vec2 GetAreaLightSize(uint lightIndex) {
    return vec2(
        u_Lights.lights[lightIndex].specificData[1],
        u_Lights.lights[lightIndex].specificData[2]
    );
}

float GetAreaLightShape(uint lightIndex) {
    return u_Lights.lights[lightIndex].specificData[3];
}

#endif
