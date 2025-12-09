// shadow_calculation.glsl
#ifndef SHADOW_CALCULATION_GLSL
#define SHADOW_CALCULATION_GLSL

#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"

/**
 * @brief 计算点光源阴影可见性
 */
float CalculatePointLightShadow(uint lightIndex, vec3 worldPos)
{
    // 获取光源位置坐标
    vec3 lightPos = u_Lights.lights[lightIndex].position;
    vec3 lightToFrag = worldPos - lightPos;
    
    // 获取该点光源在CubeMapArray中的层索引
    int shadowIndex = u_Shadow.pointShadowIndices[lightIndex].x;
    
    // 使用samplerCubeArray采样深度（标准化的线性深度）
    float closestDepth = texture(u_ShadowMapPoint, vec4(lightToFrag, shadowIndex)).r;
    
    // 添加阴影偏移避免自阴影
    float bias = u_Shadow.shadowParams.x;

    // 计算当前片段的深度（标准化到[0,1]）
    float currentDepth = length(lightToFrag + bias) / GetLightRange(lightIndex);
    
    // 检查是否在阴影中
    return currentDepth > closestDepth ? 0.0 : 1.0;
}

/**
 * @brief 计算方向光源阴影可见性
 */
float CalculateDirectionalShadow(uint lightIndex, vec3 worldPos, vec3 normal)
{
    int cascadeIndex = 0;
    
    // 确定级联索引（简化版，实际应该根据深度选择）
    float viewDepth = length(u_Camera.cameraPosition - worldPos);
    for (int i = 0; i < u_Shadow.shadowConfig.w; i++) {
        if (viewDepth <= u_Shadow.cascadeSplits[i].x) {
            cascadeIndex = i;
            break;
        }
    }
    
    // 计算阴影矩阵索引
    uint matrixIndex = lightIndex * MAX_CASCADES + cascadeIndex;
    vec4 shadowPos = u_Shadow.directionalMatrices[matrixIndex] * vec4(worldPos, 1.0);
    
    // 透视除法
    vec3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // 检查是否在阴影贴图范围内
    if (projCoords.z > 1.0 || any(lessThan(projCoords.xy, vec2(0.0))) || any(greaterThan(projCoords.xy, vec2(1.0)))) {
        return 1.0; // 超出阴影贴图范围，认为不在阴影中
    }
    
    // 获取阴影贴图层索引
    int shadowLayer = u_Shadow.directionalShadowIndices[lightIndex].x + cascadeIndex;
    
    // 采样阴影贴图
    float closestDepth = texture(u_ShadowMapDirectional, vec3(projCoords.xy, shadowLayer)).r;
    float currentDepth = projCoords.z;
    
    // 应用法线偏移
    float normalBias = u_Shadow.shadowParams.y;
    vec3 biasedWorldPos = worldPos + normal * normalBias;
    
    // 重新计算偏移后的深度
    shadowPos = u_Shadow.directionalMatrices[matrixIndex] * vec4(biasedWorldPos, 1.0);
    projCoords = shadowPos.xyz / shadowPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    currentDepth = projCoords.z;
    
    float bias = u_Shadow.shadowParams.x;
    return (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
}

/**
 * @brief 计算聚光灯阴影可见性
 */
float CalculateSpotLightShadow(uint lightIndex, vec3 worldPos)
{
    vec4 shadowPos = u_Shadow.spotLightMatrices[lightIndex] * vec4(worldPos, 1.0);
    
    vec3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // 检查是否在阴影锥体内
    if (projCoords.z > 1.0 || any(lessThan(projCoords.xy, vec2(0.0))) || any(greaterThan(projCoords.xy, vec2(1.0)))) {
        return 1.0;
    }
    
    int shadowLayer = u_Shadow.spotShadowIndices[lightIndex].x;
    float closestDepth = texture(u_ShadowMapSpot, vec3(projCoords.xy, shadowLayer)).r;
    float currentDepth = projCoords.z;
    
    float bias = u_Shadow.shadowParams.x;
    return (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
}

/**
 * @brief 计算光源阴影可见性（主函数）
 */
float CalculateShadowVisibility(uint lightIndex, vec3 worldPos, vec3 normal)
{
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    
    // 检查该光源是否有阴影
    bool hasShadow = false;
    switch (lightType) {
        case LIGHT_TYPE_POINT:
            hasShadow = (lightIndex < u_Shadow.shadowConfig.y);
            break;
        case LIGHT_TYPE_DIRECTIONAL:
            hasShadow = (lightIndex < u_Shadow.shadowConfig.x);
            break;
        case LIGHT_TYPE_SPOT:
            hasShadow = (lightIndex < u_Shadow.shadowConfig.z);
            break;
        default:
            hasShadow = false;
    }
    
    if (!hasShadow) {
        return 1.0; // 无阴影，完全可见
    }
    
    // 根据光源类型调用对应的阴影计算
    switch (lightType) {
        case LIGHT_TYPE_POINT:
            return CalculatePointLightShadow(lightIndex, worldPos);
        case LIGHT_TYPE_DIRECTIONAL:
            return CalculateDirectionalShadow(lightIndex, worldPos, normal);
        case LIGHT_TYPE_SPOT:
            return CalculateSpotLightShadow(lightIndex, worldPos);
        default:
            return 1.0;
    }
}

#endif
