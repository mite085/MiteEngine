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

void main()
{
    // // === 临时调试：显示方向光阴影贴图的级联（简洁两行布局）===
    // vec2 texCoord = fs_in.texCoord;
    // // 获取级联数量
    // int cascadeCount = u_Shadow.shadowConfig.w;
    // // 如果没有级联，显示黑色
    // if (cascadeCount == 0) {
    //     o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
    //     return;
    // }
    // // 两行布局：第一行显示级联0和1，第二行显示级联2和3
    // int maxCascadesToShow = min(cascadeCount, 4);
    // int colsPerRow = 2;
    // int rows = 2;
    // // 将屏幕分成2x2网格
    // vec2 gridCoord = texCoord * vec2(float(colsPerRow), float(rows));
    // ivec2 cell = ivec2(floor(gridCoord));
    // vec2 localCoord = fract(gridCoord);
    // // 计算级联索引
    // int cascadeIndex = cell.y * colsPerRow + cell.x;
    // // 检查是否有效
    // if (cascadeIndex >= maxCascadesToShow) {
    //     o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
    //     return;
    // }
    // // 使用第一个方向光
    // int shadowMapIndex = u_Shadow.directionalShadowIndices[0].x + cascadeIndex;
    // // 采样深度值
    // float depthValue = texture(u_ShadowMapDirectional, vec3(localCoord, shadowMapIndex)).r;
    // // 简单灰度显示
    // vec3 debugColor = vec3(depthValue);
    // // 添加边框
    // float border = 0.01;
    // if (localCoord.x < border || localCoord.x > 1.0 - border || 
    //     localCoord.y < border || localCoord.y > 1.0 - border) {
    //     debugColor = vec3(0.0); // 黑色边框
    // }
    // // 显示级联标签
    // if (localCoord.x < 0.1 && localCoord.y > 0.9) {
    //     // 级联标签颜色
    //     vec3 labelColor = vec3(1.0, 1.0, 1.0);
    //     if (cascadeIndex == 0) labelColor = vec3(1.0, 0.3, 0.3);
    //     else if (cascadeIndex == 1) labelColor = vec3(0.3, 1.0, 0.3);
    //     else if (cascadeIndex == 2) labelColor = vec3(0.3, 0.3, 1.0);
    //     else if (cascadeIndex == 3) labelColor = vec3(1.0, 1.0, 0.3);
    
    //     debugColor = labelColor;
    // }
    // o_FinalColor = vec4(debugColor, 1.0);
    // return;
    // // === 临时调试结束 ===
    
    // 读取GBuffer数据
    BRDFInput brdfInput = readGBufferData(fs_in.texCoord);
    
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


// void main()
// {
//     === 临时调试：将屏幕分成12区域显示点光源ShadowMap立方体贴图的6个面 ===
//     vec2 texCoord = fs_in.texCoord;

//     // 将屏幕分成3行4列（12个区域）
//     vec2 gridCoord = texCoord * vec2(4.0, 3.0);  // 扩展到网格坐标
//     ivec2 cell = ivec2(floor(gridCoord));        // 计算单元格索引
//     vec2 localCoord = fract(gridCoord);          // 单元格内局部坐标
    
//     // 初始化采样方向
//     vec3 sampleDir = vec3(0.0);
//     int faceLayer = 0;  // 默认使用第一组立方体贴图
    
//     // 根据单元格位置设置不同的采样方向（对应立方体贴图的6个面）
//     if (cell.y == 0) {  // 第一行：+Z面（顶部）
//         if (cell.x == 1) {  // 第一行第二列
//             // +Z面：从[0,1]映射到[-1,1]
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, 1.0 - localCoord.y * 2.0, 1.0);
//         }
//     } 
//     else if (cell.y == 1) {  // 第二行：+X, +Y, -X, -Y四个面
//         if (cell.x == 0) {      // 第二行第一列：+X面（右）
//             sampleDir = vec3(1.0, 1.0 - localCoord.y * 2.0, localCoord.x * 2.0 - 1.0);
//         }
//         else if (cell.x == 1) { // 第二行第二列：+Y面（前）
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, 1.0, 1.0 - localCoord.y * 2.0);
//         }
//         else if (cell.x == 2) { // 第二行第三列：-X面（左）
//             sampleDir = vec3(-1.0, 1.0 - localCoord.y * 2.0, 1.0 - localCoord.x * 2.0);
//         }
//         else if (cell.x == 3) { // 第二行第四列：-Y面（后）
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, -1.0, localCoord.y * 2.0 - 1.0);
//         }
//     }
//     else if (cell.y == 2) {  // 第三行：-Z面（底部）
//         if (cell.x == 1) {  // 第三行第二列
//             // -Z面：从[0,1]映射到[-1,1]
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, localCoord.y * 2.0 - 1.0, -1.0);
//         }
//     }
    
//     vec3 debugColor = vec3(0.0);
//     // 在空白区域显示黑色
//     if (sampleDir == vec3(0.0)) {
//         debugColor = vec3(0.0);
//     }else{
//         // 标准化方向向量
//         sampleDir = normalize(sampleDir);
    
//         // 采样深度值
//         float depthValue = 0.0;
//         if (sampleDir != vec3(0.0)) {  // 只在有效区域内采样
//             depthValue = texture(u_ShadowMapPoint, vec4(sampleDir, faceLayer)).r;
//         }
    
//         // 可视化深度值
//         // 添加深度值范围检查
//         if (depthValue == 0.0) {
//             // 深度为0可能表示没有写入
//             debugColor = vec3(0.0, 1.0, 0.0); // 绿色表示深度为0
//         } else if (depthValue == 1.0) {
//             // 深度为1可能表示远平面
//             debugColor = vec3(0.0, 0.0, 1.0); // 蓝色表示深度为1
//         } else {
//             // 正常深度范围
//             debugColor = vec3(depthValue);
//         }
    
//     }
    
//     o_FinalColor = vec4(debugColor, 1.0);
//     return;
//     === 临时调试结束 ===
// }


// void main()
// {
//     // === 临时调试：显示方向光阴影贴图的级联（简洁两行布局）===
//     vec2 texCoord = fs_in.texCoord;
//     // 获取级联数量
//     int cascadeCount = u_Shadow.shadowConfig.w;
//     // 如果没有级联，显示黑色
//     if (cascadeCount == 0) {
//         o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
//         return;
//     }
//     // 两行布局：第一行显示级联0和1，第二行显示级联2和3
//     int maxCascadesToShow = min(cascadeCount, 4);
//     int colsPerRow = 2;
//     int rows = 2;
//     // 将屏幕分成2x2网格
//     vec2 gridCoord = texCoord * vec2(float(colsPerRow), float(rows));
//     ivec2 cell = ivec2(floor(gridCoord));
//     vec2 localCoord = fract(gridCoord);
//     // 计算级联索引
//     int cascadeIndex = cell.y * colsPerRow + cell.x;
//     // 检查是否有效
//     if (cascadeIndex >= maxCascadesToShow) {
//         o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
//         return;
//     }
//     // 使用第一个方向光
//     int shadowMapIndex = u_Shadow.directionalShadowIndices[0].x + cascadeIndex;
//     // 采样深度值
//     float depthValue = texture(u_ShadowMapDirectional, vec3(localCoord, shadowMapIndex)).r;
//     // 简单灰度显示
//     vec3 debugColor = vec3(depthValue);
//     // 添加边框
//     float border = 0.01;
//     if (localCoord.x < border || localCoord.x > 1.0 - border || 
//         localCoord.y < border || localCoord.y > 1.0 - border) {
//         debugColor = vec3(0.0); // 黑色边框
//     }
//     // 显示级联标签
//     if (localCoord.x < 0.1 && localCoord.y > 0.9) {
//         // 级联标签颜色
//         vec3 labelColor = vec3(1.0, 1.0, 1.0);
//         if (cascadeIndex == 0) labelColor = vec3(1.0, 0.3, 0.3);
//         else if (cascadeIndex == 1) labelColor = vec3(0.3, 1.0, 0.3);
//         else if (cascadeIndex == 2) labelColor = vec3(0.3, 0.3, 1.0);
//         else if (cascadeIndex == 3) labelColor = vec3(1.0, 1.0, 0.3);
    
//         debugColor = labelColor;
//     }
//     o_FinalColor = vec4(debugColor, 1.0);
//     return;
//     // === 临时调试结束 ===
// }
