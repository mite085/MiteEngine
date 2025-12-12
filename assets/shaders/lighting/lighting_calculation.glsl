// 光照计算辅助函数
#ifndef LIGHT_CALCULATION_GLSL
#define LIGHT_CALCULATION_GLSL

#include "../common/common.glsl"
#include "../common/lights_ssbo.glsl"
#include "../common/math.glsl"

// 光照计算结果结构体
struct LightResult {
    vec3 diffuse;       // 漫反射贡献
    vec3 specular;      // 高光贡献
    float attenuation;  // 衰减系数
    float visibility;   // 可见性（阴影）
};

// 点光源衰减计算
float CalculatePointLightAttenuation(vec3 lightPosition, vec3 surfacePosition, float range, float falloff)
{
    float distance = length(lightPosition - surfacePosition);
    
    // 超出范围则完全衰减
    if (distance > range) {
        return 0.0;
    }
    
    // 物理正确的平方反比衰减
    float attenuation = 1.0 / (distance * distance);
    
    // 应用自定义衰减系数
    attenuation *= falloff;
    
    // 平滑边缘衰减
    float distanceFactor = 1.0 - smoothstep(range * 0.8, range, distance);
    
    return attenuation * distanceFactor;
}

// 聚光灯衰减计算
float CalculateSpotLightAttenuation(vec3 lightPosition, vec3 lightDirection, vec3 surfacePosition, 
                                   float innerAngle, float outerAngle, float blend, float range)
{
    vec3 toLight = lightPosition - surfacePosition;
    float distance = length(toLight);
    
    // 超出范围则完全衰减
    if (distance > range) {
        return 0.0;
    }
    
    // 距离衰减
    float distanceAttenuation = 1.0 / (distance * distance);
    
    // 角度衰减
    vec3 lightDirNormalized = normalize(toLight);
    float cosTheta = dot(lightDirNormalized, -lightDirection);
    float cosInner = cos(radians(innerAngle));
    float cosOuter = cos(radians(outerAngle));
    
    // 平滑角度衰减
    float angleAttenuation = smoothstep(cosOuter, cosInner, cosTheta);
    
    // 应用边缘柔化
    angleAttenuation = mix(angleAttenuation, 1.0, blend);
    
    return distanceAttenuation * angleAttenuation;
}

// 方向光衰减（恒定为1）
float CalculateDirectionalLightAttenuation()
{
    return 1.0;
}

// 面光源衰减计算
float CalculateAreaLightAttenuation(vec3 lightPosition, vec3 lightNormal, vec3 surfacePosition, 
                                   float power, vec2 size, float shape)
{
    // 简化处理--使用点光源近似
    float distance = length(lightPosition - surfacePosition);
    float baseAttenuation = 1.0 / (distance * distance);
    
    // 考虑面光源朝向
    vec3 toLight = normalize(lightPosition - surfacePosition);
    float cosAngle = dot(toLight, lightNormal);
    
    // 只计算正面的光照
    if (cosAngle < 0.0) {
        return 0.0;
    }
    
    // 考虑面光源尺寸
    float areaFactor = size.x * size.y;
    float normalizedAttenuation = baseAttenuation * power / areaFactor;
    
    return normalizedAttenuation;
}

// 通用光照衰减计算
float CalculateLightAttenuation(uint lightIndex, vec3 surfacePosition, vec3 surfaceNormal)
{
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    
    switch (lightType) {
        case LIGHT_TYPE_POINT:
            return CalculatePointLightAttenuation(
                light.position, surfacePosition, 
                GetLightRange(lightIndex), GetLightFalloff(lightIndex)
            );
            
        case LIGHT_TYPE_SPOT:
            return CalculateSpotLightAttenuation(
                light.position, light.direction, surfacePosition,
                GetLightInnerAngle(lightIndex), GetLightOuterAngle(lightIndex),
                GetLightBlend(lightIndex), GetLightRange(lightIndex)
            );
            
        case LIGHT_TYPE_DIRECTIONAL:
            return CalculateDirectionalLightAttenuation();
            
        case LIGHT_TYPE_AREA_RECT:
        case LIGHT_TYPE_AREA_ELLIPSE:
            return CalculateAreaLightAttenuation(
                light.position, light.direction, surfacePosition,
                GetAreaLightPower(lightIndex), GetAreaLightSize(lightIndex),
                GetAreaLightShape(lightIndex)
            );
            
        default:
            return 0.0;
    }
}

// 计算光源方向（世界空间）
vec3 CalculateLightDirection(uint lightIndex, vec3 surfacePosition)
{
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    
    switch (lightType) {
        case LIGHT_TYPE_POINT:
            return normalize(light.position - surfacePosition);
            
        case LIGHT_TYPE_SPOT:
            return normalize(light.position - surfacePosition);
            
        case LIGHT_TYPE_DIRECTIONAL:
            return -light.direction; // 方向光方向指向-Z轴
            
        case LIGHT_TYPE_AREA_RECT:
        case LIGHT_TYPE_AREA_ELLIPSE:
            // 面光源使用平均方向
            return normalize(light.position - surfacePosition);
            
        default:
            return vec3(0.0);
    }
}

// 计算光源颜色（仅涉及强度控制，暂未实现辐照度/辐射出射度/功率的控制，待后续添加）
vec3 CalculateLightColor(uint lightIndex)
{
    GPULightData light = u_Lights.lights[lightIndex];
    return light.color * light.intensity;
}

// 检查光源是否对表面可见
bool IsLightVisible(uint lightIndex, vec3 surfacePosition, vec3 surfaceNormal)
{
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    
    // 对于面光源，检查是否在正面
    if (lightType == LIGHT_TYPE_AREA_RECT || lightType == LIGHT_TYPE_AREA_ELLIPSE) {
        vec3 toLight = light.position - surfacePosition;
        float cosAngle = dot(normalize(toLight), light.direction);
        return cosAngle > 0.0;
    }
    
    // 对于其他光源，检查法线方向
    vec3 lightDir = CalculateLightDirection(lightIndex, surfacePosition);
    float NdotL = dot(surfaceNormal, lightDir);
    
    return NdotL > 0.0;
}
#endif
