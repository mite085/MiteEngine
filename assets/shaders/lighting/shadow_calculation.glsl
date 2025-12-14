// shadow_calculation.glsl
#ifndef SHADOW_CALCULATION_GLSL
#define SHADOW_CALCULATION_GLSL

#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"

/**
 * @brief 获取光源的类型内局部索引
 */
uint GetLightTypeLocalIndex(uint lightIndex) {
    return uint(u_Lights.lights[lightIndex].typeLocalIndex);
}
/**
 * @brief 获取方向光源阴影矩阵索引
 */
int GetDirectionalShadowMatrixIndex(uint lightIndex) {
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    return int(localIndex * u_Shadow.shadowConfig.w); // 乘以级联数量
}
/**
 * @brief 获取点光源阴影矩阵索引
 */
int GetPointShadowMatrixIndex(uint lightIndex) {
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    return int(localIndex * 6); // 每个点光源6个面
}
/**
 * @brief 获取聚光灯阴影矩阵索引
 */
int GetSpotShadowMatrixIndex(uint lightIndex) {
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    return int(localIndex);
}

/**
 * @brief 检查光源是否有阴影
 */
bool HasShadow(uint lightIndex) {
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    
    // 检查是否在有效范围内
    switch (lightType) {
        case LIGHT_TYPE_DIRECTIONAL:
            return localIndex < uint(u_Shadow.shadowConfig.x);
        case LIGHT_TYPE_POINT:
            return localIndex < uint(u_Shadow.shadowConfig.y);
        case LIGHT_TYPE_SPOT:
            return localIndex < uint(u_Shadow.shadowConfig.z);
        default:
            return false;
    }
}

/**
 * @brief 计算点光源阴影可见性（基于分辨率的PCF）
 */
float CalculatePointLightShadow(uint lightIndex, vec3 worldPos)
{
    vec3 lightPos = u_Lights.lights[lightIndex].position;
    vec3 lightToFrag = worldPos - lightPos;
    float distance = length(lightToFrag);
    
    // 使用类型内索引获取阴影索引
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    int shadowIndex = u_Shadow.pointShadowIndices[localIndex].x;

    // 计算当前深度值
    float bias = u_Shadow.shadowParams.x;
    float currentDepth = (distance - bias) / GetLightRange(lightIndex);
    
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
 * @brief 计算方向光源阴影可见性（基于分辨率的PCF）
 */
float CalculateDirectionalShadow(uint lightIndex, vec3 worldPos, vec3 normal)
{
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    uint cascadeCount = uint(u_Shadow.shadowConfig.w);
    
    // 确定使用哪个级联
    uint cascadeIndex = 0;
    float depth = length(worldPos - u_Camera.cameraPosition);
    
    for (uint i = 0; i < cascadeCount - 1; ++i) {
        if (depth < u_Shadow.cascadeSplits[i].x) {
            cascadeIndex = i;
            break;
        }
    }
    
    // 计算矩阵索引
    int matrixIndex = GetDirectionalShadowMatrixIndex(lightIndex) + int(cascadeIndex);
    
    vec4 shadowPos = u_Shadow.directionalMatrices[matrixIndex] * vec4(worldPos, 1.0);
    shadowPos.xyz /= shadowPos.w;
    vec3 projCoords = shadowPos.xyz * 0.5 + 0.5;
    
    if (projCoords.z > 1.0) return 1.0;
    
    // 使用类型内索引获取阴影层
    int shadowLayer = u_Shadow.directionalShadowIndices[localIndex].x + int(cascadeIndex);
    
    // 计算基于阴影贴图分辨率的PCF偏移
    float texelSize = 1.0 / u_Shadow.shadowParams.w; // 阴影贴图尺寸
    
    // 3x3 PCF采样
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 sampleCoords = projCoords.xy + offset;
            
            float closestDepth = texture(u_ShadowMapDirectional, vec3(sampleCoords, shadowLayer)).r;
            float currentDepth = projCoords.z;
            
            // 添加偏移以减少阴影痤疮
            float bias = u_Shadow.shadowParams.x;
            shadow += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
        }
    }
    
    shadow /= 9.0;
    
    return shadow;
}

/**
 * @brief 计算聚光灯阴影可见性
 */
float CalculateSpotLightShadow(uint lightIndex, vec3 worldPos)
{
    uint localIndex = GetLightTypeLocalIndex(lightIndex);
    int matrixIndex = GetSpotShadowMatrixIndex(lightIndex);
    
    vec4 shadowPos = u_Shadow.spotLightMatrices[matrixIndex] * vec4(worldPos, 1.0);
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
    if (!HasShadow(lightIndex)) {
        return 1.0; // 无阴影，完全可见
    }
    
    GPULightData light = u_Lights.lights[lightIndex];
    uint lightType = uint(light.type);
    
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
