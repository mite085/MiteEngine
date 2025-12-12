// shadow_calculation.glsl
#ifndef SHADOW_CALCULATION_GLSL
#define SHADOW_CALCULATION_GLSL

#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"

/**
 * @brief 计算点光源阴影可见性（基于分辨率的PCF）
 */
float CalculatePointLightShadow(uint lightIndex, vec3 worldPos)
{
    vec3 lightPos = u_Lights.lights[lightIndex].position;
    vec3 lightToFrag = worldPos - lightPos;
    float distance = length(lightToFrag);
    
    int shadowIndex = u_Shadow.pointShadowIndices[lightIndex].x;
    float bias = u_Shadow.shadowParams.x;
    float currentDepth = (distance + bias) / GetLightRange(lightIndex);
    
    // 基于阴影贴图分辨率计算PCF半径
    // 假设阴影贴图分辨率为1024，每个纹素的角度大约为1/1024 rad
    float shadowMapSize = u_Shadow.shadowParams.w; // 从UBO获取阴影贴图尺寸
    float texelAngle = 1.0 / shadowMapSize; // 每个纹素对应的角度
    
    // 将角度转换为方向偏移
    // 距离光源越远，相同的角度偏移对应的空间偏移越大
    float pcfRadius = texelAngle * 0.5; // 使用半个纹素的偏移
    
    // 3x3 PCF（9次采样）
    float shadow = 0.0;
    
    vec3 sampleDir = normalize(lightToFrag);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, sampleDir));
    vec3 forward = normalize(cross(right, sampleDir));
    
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec3 sampleOffset = right * (float(x) * pcfRadius) + 
                               forward * (float(y) * pcfRadius);
            vec3 sampleVector = normalize(sampleDir + sampleOffset);
            
            float closestDepth = texture(u_ShadowMapPoint, vec4(sampleVector, shadowIndex)).r;
            shadow += currentDepth > closestDepth ? 0.0 : 1.0;
        }
    }
    
    shadow /= 9.0;
    
    return shadow;
}

/**
 * @brief 计算方向光源阴影可见性
 */
float CalculateDirectionalShadow(uint lightIndex, vec3 worldPos, vec3 normal)
{
    // 极简版本：假设所有数据都已正确设置
    
    // 1. 使用第0个阴影矩阵
    vec4 shadowPos = u_Shadow.directionalMatrices[lightIndex] * vec4(worldPos, 1.0);
    
    // 2. 转换到纹理空间
    shadowPos.xyz /= shadowPos.w;
    vec3 projCoords = shadowPos.xyz * 0.5 + 0.5;
    
    // 3. 简单范围检查
    if (projCoords.z > 1.0) return 1.0;
    
    // 4. 采样并比较
    float closestDepth = texture(u_ShadowMapDirectional, vec3(projCoords.xy, lightIndex)).r;
    float currentDepth = projCoords.z;
    
    // 5. 硬阴影
    return currentDepth > closestDepth ? 0.0 : 1.0;
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
    int shadowMapIndex = -1;

    switch (lightType) {
        case LIGHT_TYPE_POINT:
            // 添加边界检查
            if (lightIndex < MAX_LIGHTS) {
                shadowMapIndex = u_Shadow.pointShadowIndices[lightIndex].x;
                hasShadow = (shadowMapIndex >= 0);
            }
            break;
        case LIGHT_TYPE_DIRECTIONAL:
            // 添加边界检查
            if (lightIndex < MAX_LIGHTS) {
                shadowMapIndex = u_Shadow.directionalShadowIndices[lightIndex].x;
                hasShadow = (shadowMapIndex >= 0);
            }
            break;
        case LIGHT_TYPE_SPOT:
            // 添加边界检查
            if (lightIndex < MAX_LIGHTS) {
                shadowMapIndex = u_Shadow.spotShadowIndices[lightIndex].x;
                hasShadow = (shadowMapIndex >= 0);
            }
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
