// 自发光BRDF实现
#ifndef EMISSION_BRDF_GLSL
#define EMISSION_BRDF_GLSL

#include "brdf_common.glsl"

/**
 * @brief 自发光材质直接光照计算
 * @note 自发光材质不受直接光照影响，只显示自发光
 */
BRDFResult calculateEmissionDirectBRDF(BRDFInput brdfInput, BRDFLightInput lightInput)
{
    BRDFResult result;
    
    // 自发光材质不受直接光照影响
    result.diffuse = vec3(0.0);
    result.specular = vec3(0.0);
    
    // 自发光 = 基础颜色 * 自发光强度
    float emissionIntensity = brdfInput.materialParams.w;
    result.emission = brdfInput.baseColor * emissionIntensity;
    result.alpha = 1.0;
    
    return result;
}

/**
 * @brief 自发光材质环境光照计算
 */
BRDFResult calculateEmissionAmbientBRDF(BRDFInput brdfInput, BRDFAmbientInput ambientInput)
{
    BRDFResult result;
    
    // 自发光材质不受环境光影响
    result.diffuse = vec3(0.0);
    result.specular = vec3(0.0);
    
    // 自发光 = 基础颜色 * 自发光强度
    float emissionIntensity = brdfInput.materialParams.w;
    result.emission = brdfInput.baseColor * emissionIntensity;
    result.alpha = 1.0;
    
    return result;
}

#endif
