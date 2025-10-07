// BRDF分发器 - 根据材质类型调用不同的BRDF实现
#ifndef BRDF_DISPATCHER_GLSL
#define BRDF_DISPATCHER_GLSL

#include "brdf_common.glsl"

// 包含所有BRDF实现
#include "pbr.brdf.glsl"
#include "lambert.brdf.glsl" 
#include "emission.brdf.glsl"

/**
 * @brief 根据材质类型分发到对应的BRDF计算
 */
BRDFResult dispatchBRDF(BRDFInput brdfInput, BRDFLightInput lightInput)
{
    // 材质类型相关的GBuffer已经是最邻近采样，不存在材质1和材质3线性混合为材质2的情况
    // 但此处为了避免uint()的向下取整导致错位，使用round进行四舍五入
    uint materialType = uint(round(brdfInput.materialType));
    
    switch (materialType) {
        case 0: // PBR
            return calculatePBRDirectBRDF(brdfInput, lightInput);
        case 1: // Lambert
            return calculateLambertDirectBRDF(brdfInput, lightInput);
        case 2: // Emission
            return calculateEmissionDirectBRDF(brdfInput, lightInput);
        default:
            // 回退到PBR
            return calculatePBRDirectBRDF(brdfInput, lightInput);
    }
}

/**
 * @brief 根据材质类型分发到对应的环境BRDF计算
 */
BRDFResult dispatchAmbientBRDF(BRDFInput brdfInput, BRDFAmbientInput ambientInput)
{
    uint materialType = uint(round(brdfInput.materialType));
    
    switch (materialType) {
        case 0: // PBR
            return calculatePBRAmbientBRDF(brdfInput, ambientInput);
        case 1: // Lambert
            return calculateLambertAmbientBRDF(brdfInput, ambientInput);
        case 2: // Emission
            return calculateEmissionAmbientBRDF(brdfInput, ambientInput);
        default:
            // 回退到PBR
            return calculatePBRAmbientBRDF(brdfInput, ambientInput);
    }
}

#endif
