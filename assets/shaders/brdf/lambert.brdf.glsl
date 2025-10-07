// Lambert漫反射BRDF实现
#ifndef LAMBERT_BRDF_GLSL
#define LAMBERT_BRDF_GLSL

#include "brdf_common.glsl"

/**
 * @brief Lambert漫反射直接光照计算
 */
BRDFResult calculateLambertDirectBRDF(BRDFInput brdfInput, BRDFLightInput lightInput)
{
    BRDFResult result;
    
    // 标准化向量
    vec3 N = normalize(brdfInput.normal);
    vec3 L = normalize(lightInput.lightDirection);
    
    // 计算NdotL
    float NdotL = max(dot(N, L), 0.0);
    
    // Lambert漫反射：baseColor * NdotL / PI
    vec3 diffuse = brdfInput.baseColor * NdotL / PI;
    
    // 应用光照
    vec3 radiance = lightInput.lightColor * lightInput.attenuation * lightInput.visibility;
    
    result.diffuse = diffuse * radiance;
    result.specular = vec3(0.0); // Lambert没有镜面反射
    result.emission = brdfInput.emission;
    result.alpha = 1.0;
    
    return result;
}

/**
 * @brief Lambert环境光照计算
 */
BRDFResult calculateLambertAmbientBRDF(BRDFInput brdfInput, BRDFAmbientInput ambientInput)
{
    BRDFResult result;
    
    // Lambert环境光：baseColor * irradiance
    vec3 ambient = brdfInput.baseColor * ambientInput.irradiance;
    
    // 应用环境光遮蔽和强度
    result.diffuse = ambient * brdfInput.occlusion * ambientInput.ambientIntensity;
    result.specular = vec3(0.0);
    result.emission = brdfInput.emission;
    result.alpha = 1.0;
    
    return result;
}

#endif
