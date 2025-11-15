// 延迟光照全屏四边形片段着色器
#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec2 texCoord;          // 纹理坐标
    layout(location = 1) vec3 viewRay;           // 视图射线
} fs_in;

// 最终输出颜色
layout(location = 0) out vec4 o_FinalColor;

// 包含必要的头文件
#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/math.glsl"
#include "../common/lights_ssbo.glsl"
#include "../lighting/lighting_calculation.glsl"
#include "../lighting/shadow_calculation.glsl"
#include "../lighting/ambient_calculation.glsl"
#include "../brdf/brdf_dispatcher.glsl"

/**
 * @brief 从GBuffer重建世界空间位置
 * @param texCoord 纹理坐标
 * @param viewRay 视图射线
 * @return 世界空间位置
 */
vec3 reconstructWorldPosition(vec2 texCoord, vec3 viewRay)
{
    // 方法1--直接从GBuffer0读取存储的世界位置（更准确）
    vec4 worldPosDepth = texture(u_GBufferWorldPosDepth, texCoord);
    
    // 如果GBuffer中存储了有效的世界位置，直接使用
    if (length(worldPosDepth.xyz) > 0.0) {
        return worldPosDepth.xyz;
    }
    
    // 方法2--通过深度和视图射线重建（备用方法）
    float depth = texture(u_GBufferWorldPosDepth, texCoord).a;
    
    if (u_Camera.projectionType == PROJECTION_PERSPECTIVE) {
        // 透视投影重建
        vec3 viewPos = viewRay * depth;
        vec4 worldPos = inverse(u_Camera.view) * vec4(viewPos, 1.0);
        return worldPos.xyz;
    } else {
        // 正交投影重建
        vec3 viewPos = vec3(viewRay.xy, depth);
        vec4 worldPos = inverse(u_Camera.view) * vec4(viewPos, 1.0);
        return worldPos.xyz;
    }
}

/**
 * @brief 从GBuffer读取所有表面属性
 * @param texCoord 纹理坐标
 * @return BRDF输入参数
 */
BRDFInput readGBufferData(vec2 texCoord)
{
    BRDFInput brdfInput;
    
    // 重建世界位置
    brdfInput.worldPosition = reconstructWorldPosition(texCoord, fs_in.viewRay);
    
    // 读取法线（从GBuffer3）
    vec4 normalScale = texture(u_GBufferNormalScale, texCoord);
    brdfInput.normal = unpackNormal(normalScale.xyz);
    
    // 计算视线方向
    brdfInput.viewDirection = normalize(u_Camera.cameraPosition - brdfInput.worldPosition);
    
    // 读取基础颜色和材质类型（GBuffer1）
    vec4 baseColorMatType = texture(u_GBufferBaseColorMatType, texCoord);
    brdfInput.baseColor = baseColorMatType.rgb;
    // 材质类型需要四舍五入消除偏移影响（GBuffer均为最邻近采样，避免材质1和材质3的边界被识别为材质2的情况）
    brdfInput.materialType = uint(round(baseColorMatType.a));
    
    // 读取金属度、粗糙度和AO（GBuffer2）
    vec4 metallicRoughnessAO = texture(u_GBufferMetallicRoughnessAO, texCoord);
    brdfInput.metallic = metallicRoughnessAO.r;
    brdfInput.roughness = metallicRoughnessAO.g;
    brdfInput.occlusion = metallicRoughnessAO.b;
    
    // 读取自发光（GBuffer4）
    vec4 emissionAlpha = texture(u_GBufferEmissionAlpha, texCoord);
    brdfInput.emission = emissionAlpha.rgb;
    
    return brdfInput;
}

/**
 * @brief 准备光源输入参数
 * @param lightIndex 光源索引
 * @param brdfInput BRDF输入参数
 * @return 光源输入参数
 */
BRDFLightInput prepareLightInput(uint lightIndex, BRDFInput brdfInput)
{
    BRDFLightInput lightInput;
    
    // 获取光源数据
    GPULightData light = u_Lights.lights[lightIndex];
    
    // 基础光源属性
    lightInput.lightColor = CalculateLightColor(lightIndex);
    lightInput.lightDirection = CalculateLightDirection(lightIndex, brdfInput.worldPosition);
    lightInput.attenuation = CalculateLightAttenuation(lightIndex, brdfInput.worldPosition, brdfInput.normal);

    // 计算阴影可见性
    lightInput.visibility = CalculateShadowVisibility(lightIndex, brdfInput.worldPosition, brdfInput.normal);
    
    // 光源类型特定数据
    lightInput.lightType = uint(light.type);
    lightInput.lightPosition = light.position;
    lightInput.lightRange = GetLightRange(lightIndex);
    
    return lightInput;
}

/**
 * @brief 计算单个光源的贡献
 * @param lightIndex 光源索引
 * @param brdfInput BRDF输入参数
 * @return 光照贡献
 */
vec3 calculateLightContribution(uint lightIndex, BRDFInput brdfInput)
{
    // 检查光源是否可见
    if (!IsLightVisible(lightIndex, brdfInput.worldPosition, brdfInput.normal)) {
        return vec3(0.0);
    }

    // 准备光源输入
    BRDFLightInput lightInput = prepareLightInput(lightIndex, brdfInput);
    
    // 如果完全在阴影中，跳过BRDF计算
    if (lightInput.visibility <= 0.0) {
        return vec3(0.0);
    }
    
    // 使用BRDF分发器计算光照
    BRDFResult result = dispatchBRDF(brdfInput, lightInput);
    
    // 应用阴影到光照结果
    return (result.diffuse + result.specular) * lightInput.visibility;
}

/**
 * @brief 计算环境光照贡献
 * @param brdfInput BRDF输入参数
 * @return 环境光照贡献
 */
vec3 calculateAmbientContribution(BRDFInput brdfInput)
{
    // 准备环境光照输入
    BRDFAmbientInput ambientInput = prepareAmbientInputApprox(brdfInput, u_EnvironmentMap);
    
    // 使用BRDF分发器计算环境光照
    BRDFResult result = dispatchAmbientBRDF(brdfInput, ambientInput);
    
    // 返回环境光照贡献
    return result.diffuse + result.specular;
}

/**
 * @brief 调试显示函数
 * @param brdfInput BRDF输入参数
 * @param texCoord 纹理坐标
 * @return 调试颜色
 */
// vec3 debugDisplay(BRDFInput brdfInput, vec2 texCoord)
// {
//     switch (u_DebugMode) {
//         case 1: // 显示世界位置
//             return brdfInput.worldPosition * 0.1 + 0.5;
//         case 2: // 显示法线
//             return brdfInput.normal * 0.5 + 0.5;
//         case 3: // 显示基础颜色
//             return brdfInput.baseColor;
//         case 4: // 显示金属度
//             return vec3(brdfInput.metallic);
//         case 5: // 显示粗糙度
//             return vec3(brdfInput.roughness);
//         case 6: // 显示AO
//             return vec3(brdfInput.occlusion);
//         case 7: // 显示自发光
//             return brdfInput.emission;
//         case 8: // 显示材质类型
//             return (brdfInput.materialType == MATERIAL_TYPE_PBR) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
//         case 9: // 显示单个光源
//             if (u_DebugLightIndex < u_Lights.header.lightCount) {
//                 return calculateLightContribution(uint(u_DebugLightIndex), brdfInput);
//             }
//             return vec3(0.0);
//         case 10: // 显示环境光照
//             return calculateAmbientContribution(brdfInput);
//         default:
//             return vec3(0.0);
//     }
// }

void main()
{
    // 读取GBuffer数据
    BRDFInput brdfInput = readGBufferData(fs_in.texCoord);
    
    // 调试模式显示
    //if (u_DebugMode > 0) {
    //    vec3 debugColor = debugDisplay(brdfInput, fs_in.texCoord);
    //    o_FinalColor = vec4(debugColor, 1.0);
    //    return;
    //}
    
    // 初始化最终颜色
    vec3 finalColor = vec3(0.0);
    
    // 计算环境光照贡献
    vec3 ambient = calculateAmbientContribution(brdfInput);
    finalColor += ambient;
    
    // 计算所有直接光源贡献
    for (uint i = 0u; i < u_Lights.header.lightCount; i++) {
        vec3 lightContribution = calculateLightContribution(i, brdfInput);
        finalColor += lightContribution;
    }
    
    // 添加自发光
    finalColor += brdfInput.emission;
    
    // 应用色调映射（简单Reinhard）
    finalColor = finalColor / (finalColor + vec3(1.0));
    
    // 输出最终颜色
    o_FinalColor = vec4(finalColor, 1.0);
}
