// 基于GLTF标准的PBR材质BRDF实现
#ifndef PBR_BRDF_GLSL
#define PBR_BRDF_GLSL

#include "brdf_common.glsl"

// =========================================================================
// PBR BRDF核心函数实现
// =========================================================================

/**
 * @brief 计算基础反射率F0（符合GLTF标准）
 * @param baseColor 基础颜色（线性空间）
 * @param metallic 金属度
 * @return 基础反射率F0
 */
vec3 calculateF0(vec3 baseColor, float metallic)
{
    // 电介质的基础反射率约为0.04
    vec3 dielectricF0 = vec3(0.04);
    
    // 金属的基础反射率等于其基础颜色
    // 根据金属度在电介质和金属之间插值
    return mix(dielectricF0, baseColor, metallic);
}

/**
 * @brief 验证BRDF输入参数的合法性
 */
bool validateBRDFInput(BRDFInput input)
{
    // 检查基础参数范围
    if (any(lessThan(input.baseColor, vec3(0.0))) return false;
    if (input.metallic < 0.0 || input.metallic > 1.0) return false;
    if (input.roughness < 0.0 || input.roughness > 1.0) return false;
    if (input.occlusion < 0.0 || input.occlusion > 1.0) return false;
    
    // 检查向量合法性
    if (length(input.normal) < EPSILON) return false;
    if (length(input.viewDirection) < EPSILON) return false;
    
    return true;
}

/**
 * @brief 准备BRDF计算所需的中间参数
 */
BRDFIntermediate prepareBRDFIntermediate(BRDFInput brdfInput, BRDFLightInput lightInput)
{
    BRDFIntermediate intermediate;
    
    // 标准化向量
    vec3 N = normalize(brdfInput.normal);
    vec3 V = normalize(brdfInput.viewDirection);
    vec3 L = normalize(lightInput.lightDirection);
    intermediate.H = normalize(V + L);
    
    // 计算点积
    intermediate.NdotL = max(dot(N, L), 0.0);
    intermediate.NdotV = max(dot(N, V), 0.0);
    intermediate.NdotH = max(dot(N, intermediate.H), 0.0);
    intermediate.VdotH = max(dot(V, intermediate.H), 0.0);
    intermediate.LdotH = max(dot(L, intermediate.H), 0.0);
    
    // 计算基础反射率
    intermediate.F0 = calculateF0(brdfInput.baseColor, brdfInput.metallic);
    
    // 计算菲涅尔项（Schlick近似）
    intermediate.F = calculateFresnelSchlick(intermediate.VdotH, intermediate.F0);
    
    // 计算法线分布函数（GGX/Trowbridge-Reitz）
    intermediate.D = calculateDistributionGGX(N, intermediate.H, brdfInput.roughness);
    
    // 计算几何遮蔽函数（Smith方法）
    intermediate.G = calculateGeometrySmith(N, V, L, brdfInput.roughness);
    
    return intermediate;
}

/**
 * @brief 计算漫反射BRDF项（Disney diffuse）
 */
vec3 calculateDiffuseBRDF(BRDFInput brdfInput, BRDFIntermediate intermediate)
{
    // 使用Disney漫反射模型，考虑粗糙度对漫反射的影响
    float FD90 = 0.5 + 2.0 * intermediate.LdotH * intermediate.LdotH * brdfInput.roughness;
    float FdV = 1.0 + (FD90 - 1.0) * pow(1.0 - intermediate.NdotV, 5.0);
    float FdL = 1.0 + (FD90 - 1.0) * pow(1.0 - intermediate.NdotL, 5.0);
    
    // 基础漫反射颜色（金属没有漫反射）
    vec3 diffuseColor = brdfInput.baseColor * (1.0 - brdfInput.metallic);
    
    return diffuseColor * (FdV * FdL / PI);
}

/**
 * @brief 计算镜面反射BRDF项（Cook-Torrance）
 */
vec3 calculateSpecularBRDF(BRDFInput brdfInput, BRDFIntermediate intermediate)
{
    // Cook-Torrance BRDF公式: (F * D * G) / (4 * NdotL * NdotV)
    float denominator = 4.0 * intermediate.NdotL * intermediate.NdotV + EPSILON;
    vec3 specular = (intermediate.F * intermediate.D * intermediate.G) / denominator;
    
    return specular;
}

/**
 * @brief 计算能量守恒的光照贡献
 */
BRDFResult calculateEnergyConservedBRDF(BRDFInput brdfInput, BRDFLightInput lightInput, BRDFIntermediate intermediate)
{
    BRDFResult result;
    
    // 计算漫反射和镜面反射
    vec3 diffuseBRDF = calculateDiffuseBRDF(brdfInput, intermediate);
    vec3 specularBRDF = calculateSpecularBRDF(brdfInput, intermediate);
    
    // 能量守恒：镜面反射和漫反射不能同时达到最大值
    // 金属材质没有漫反射，电介质材质镜面反射较弱
    vec3 kS = intermediate.F;                    // 镜面反射比例
    vec3 kD = vec3(1.0) - kS;                   // 漫反射比例
    kD *= 1.0 - brdfInput.metallic;             // 金属没有漫反射
    
    // 应用光照
    vec3 radiance = lightInput.lightColor * lightInput.attenuation * lightInput.visibility;
    
    // 最终光照贡献
    result.diffuse = kD * diffuseBRDF * radiance * intermediate.NdotL;
    result.specular = specularBRDF * radiance * intermediate.NdotL;
    result.emission = brdfInput.emission;       // 自发光独立计算
    result.alpha = 1.0;                         // PBR材质默认不透明
    
    return result;
}

/**
 * @brief 计算直接光照的BRDF贡献（主函数）
 */
BRDFResult calculateDirectBRDF(BRDFInput brdfInput, BRDFLightInput lightInput)
{
    // 验证输入参数
    if (!validateBRDFInput(brdfInput)) {
        return BRDFResult(vec3(0.0), vec3(0.0), brdfInput.emission, 1.0);
    }
    
    // 检查光照可见性
    if (lightInput.visibility <= 0.0 || lightInput.attenuation <= 0.0) {
        return BRDFResult(vec3(0.0), vec3(0.0), brdfInput.emission, 1.0);
    }
    
    // 准备中间参数
    BRDFIntermediate intermediate = prepareBRDFIntermediate(brdfInput, lightInput);
    
    // 检查光照方向是否有效
    if (intermediate.NdotL <= 0.0) {
        return BRDFResult(vec3(0.0), vec3(0.0), brdfInput.emission, 1.0);
    }
    
    // 计算能量守恒的BRDF
    return calculateEnergyConservedBRDF(brdfInput, lightInput, intermediate);
}

/**
 * @brief 计算环境光照的BRDF贡献（IBL）
 */
BRDFResult calculateAmbientBRDF(BRDFInput brdfInput, BRDFAmbientInput ambientInput)
{
    BRDFResult result;
    
    // 标准化向量
    vec3 N = normalize(brdfInput.normal);
    vec3 V = normalize(brdfInput.viewDirection);
    float NdotV = max(dot(N, V), 0.0);
    
    // 计算基础反射率
    vec3 F0 = calculateF0(brdfInput.baseColor, brdfInput.metallic);
    
    // 环境光漫反射项
    vec3 kS = calculateFresnelSchlickRoughness(NdotV, F0, brdfInput.roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - brdfInput.metallic;
    
    vec3 diffuseIBL = kD * brdfInput.baseColor * ambientInput.irradiance;
    
    // 环境光镜面反射项
    vec3 F = calculateFresnelSchlickRoughness(NdotV, F0, brdfInput.roughness);
    vec2 envBRDF = ambientInput.brdfLUT.rg;
    vec3 specularIBL = ambientInput.prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    // 应用环境光遮蔽和环境光强度
    float ambientOcclusion = brdfInput.occlusion;
    float ambientStrength = ambientInput.ambientIntensity;
    
    result.diffuse = diffuseIBL * ambientOcclusion * ambientStrength;
    result.specular = specularIBL * ambientOcclusion * ambientStrength;
    result.emission = brdfInput.emission;
    result.alpha = 1.0;
    
    return result;
}

#endif
