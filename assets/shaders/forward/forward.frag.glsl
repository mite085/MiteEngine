// 前向渲染片段着色器 - 专用于半透明物体渲染
#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec3 worldPos;          // 世界空间位置
    layout(location = 1) vec3 normal;            // 世界空间法线
    layout(location = 2) vec3 tangent;           // 世界空间切线
    layout(location = 3) vec3 bitangent;         // 世界空间副切线
    layout(location = 4) vec2 texCoord;          // 纹理坐标
    // layout(location = 5) vec4 clipPos;           // 半透明物体不写入深度，无需传递裁剪空间位置
} fs_in;

// 最终输出颜色（用于Alpha混合）
layout(location = 0) out vec4 o_ForwardColor;

// 包含必要的头文件
#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"
#include "../common/math.glsl"
#include "../lighting/lighting_calculation.glsl"
#include "../lighting/shadow_calculation.glsl"
#include "../lighting/ambient_calculation.glsl"
#include "../brdf/brdf_dispatcher.glsl"

/**
 * @brief 应用纹理坐标变换
 * @param texCoord 原始纹理坐标
 * @param params 纹理参数（xy:缩放, zw:偏移）
 * @return 变换后的纹理坐标
 */
vec2 applyTextureTransform(vec2 texCoord, vec4 params)
{
    return texCoord * params.xy + params.zw;
}

/**
 * @brief 采样基础颜色纹理
 * @param texCoord 纹理坐标
 * @return 基础颜色（线性空间）
 */
vec4 sampleBaseColor(vec2 texCoord)
{
    vec4 baseColor = u_Material.baseColor;
    
    if (u_Material.textureCNMROFlags.x > 0.5) {
        vec2 transformedCoord = applyTextureTransform(texCoord, u_Material.baseColorTexParams);
        baseColor = texture(u_BaseColorTexture, transformedCoord);
        
        // sRGB到线性空间转换
        baseColor.rgb = pow(baseColor.rgb, vec3(2.2));
    }
    
    return baseColor;
}

/**
 * @brief 计算法线（考虑法线贴图）
 * @param texCoord 纹理坐标
 * @return 世界空间法线
 */
vec3 calculateNormal(vec2 texCoord)
{
    vec3 normal = normalize(fs_in.normal);
    
    if (u_Material.textureCNMROFlags.y > 0.5) {
        // 采样法线贴图
        vec2 transformedCoord = applyTextureTransform(texCoord, u_Material.normalTexParams);
        vec3 tangentNormal = unpackNormal(texture(u_NormalTexture, transformedCoord).rgb);
        
        // 构建TBN矩阵
        mat3 TBN = mat3(
            normalize(fs_in.tangent),
            normalize(fs_in.bitangent),
            normalize(fs_in.normal)
        );
        
        // 变换到世界空间
        vec3 mappedNormal = normalize(TBN * tangentNormal);
        
        // 应用法线贴图强度
        if (u_Material.normalScale.x < 1.0) {
            normal = normalize(mix(normal, mappedNormal, u_Material.normalScale.x));
        } else {
            normal = mappedNormal;
        }
    }
    
    // 双面渲染处理（半透明物体通常需要双面渲染）
    if (u_Material.renderProperties.y > 0.5) {
        vec3 viewDir = normalize(u_Camera.cameraPosition - fs_in.worldPos);
        if (dot(normal, viewDir) < 0.0) {
            normal = -normal;
        }
    }
    
    return normal;
}

/**
 * @brief 采样金属度和粗糙度
 * @param texCoord 纹理坐标
 * @return vec2(金属度, 粗糙度)
 */
vec2 sampleMetallicRoughness(vec2 texCoord)
{
    vec2 metallicRoughness = u_Material.metallicRoughnessAO.xy;
    
    if (u_Material.textureCNMROFlags.z > 0.5) {
        vec2 transformedCoord = applyTextureTransform(texCoord, u_Material.mrTexParams);
        vec4 mrSample = texture(u_MetallicRoughnessTexture, transformedCoord);
        
        // GLTF标准--金属度在B通道，粗糙度在G通道
        metallicRoughness.r = mrSample.b; // 金属度
        metallicRoughness.g = mrSample.g; // 粗糙度
    }
    
    return metallicRoughness;
}

/**
 * @brief 采样环境光遮蔽
 * @param texCoord 纹理坐标
 * @return 环境光遮蔽值
 */
float sampleOcclusion(vec2 texCoord)
{
    float occlusion = u_Material.metallicRoughnessAO.z;
    
    if (u_Material.textureCNMROFlags.w > 0.5) {
        vec2 transformedCoord = applyTextureTransform(texCoord, u_Material.occlusionTexParams);
        occlusion = texture(u_OcclusionTexture, transformedCoord).r;
    }
    
    return occlusion;
}

/**
 * @brief 采样自发光
 * @param texCoord 纹理坐标
 * @return 自发光颜色
 */
vec3 sampleEmission(vec2 texCoord)
{
    vec3 emission = u_Material.emission.rgb * u_Material.emission.a;
    
    if (u_Material.textureEmissionFlag.x > 0.5) {
        vec2 transformedCoord = applyTextureTransform(texCoord, u_Material.emissiveTexParams);
        vec3 emissiveSample = texture(u_EmissiveTexture, transformedCoord).rgb;
        
        // sRGB到线性空间转换
        emission = pow(emissiveSample, vec3(2.2)) * u_Material.emission.a;
    }
    
    return emission;
}

/**
 * @brief 准备BRDF输入参数
 * @return BRDF输入参数
 */
BRDFInput prepareBRDFInput()
{
    BRDFInput brdfInput;
    
    // 基础几何信息
    brdfInput.worldPosition = fs_in.worldPos;
    brdfInput.normal = calculateNormal(fs_in.texCoord);
    brdfInput.viewDirection = normalize(u_Camera.cameraPosition - fs_in.worldPos);
    
    // 采样材质属性
    vec4 baseColor = sampleBaseColor(fs_in.texCoord);
    brdfInput.baseColor = baseColor.rgb;
    
    vec2 metallicRoughness = sampleMetallicRoughness(fs_in.texCoord);
    brdfInput.metallic = metallicRoughness.r;
    brdfInput.roughness = metallicRoughness.g;
    
    brdfInput.occlusion = sampleOcclusion(fs_in.texCoord);
    brdfInput.emission = sampleEmission(fs_in.texCoord);
    
    // 材质类型（复用GBuffer的材质类型定义）
    brdfInput.materialType = uint(round(u_Material.materialInfo.x));
    
    return brdfInput;
}

/**
 * @brief 准备光源输入参数（复用deferred_lighting的逻辑）
 */
BRDFLightInput prepareLightInput(uint lightIndex, BRDFInput brdfInput)
{
    BRDFLightInput lightInput;
    
    // 获取光源数据
    GPULightData light = u_Lights.lights[lightIndex];
    
    // 基础光源属性（复用deferred_lighting的函数）
    lightInput.lightColor = CalculateLightColor(lightIndex);
    lightInput.lightDirection = CalculateLightDirection(lightIndex, brdfInput.worldPosition);
    lightInput.attenuation = CalculateLightAttenuation(lightIndex, brdfInput.worldPosition, brdfInput.normal);
    
    // 阴影可见性（半透明物体接收阴影）
    lightInput.visibility = CalculateShadowVisibility(lightIndex, brdfInput.worldPosition, brdfInput.normal);
    
    // 光源类型特定数据
    lightInput.lightType = uint(light.type);
    lightInput.lightPosition = light.position;
    lightInput.lightRange = GetLightRange(lightIndex);
    
    return lightInput;
}

/**
 * @brief 计算单个光源的贡献（复用deferred_lighting的逻辑）
 */
vec3 calculateLightContribution(uint lightIndex, BRDFInput brdfInput)
{
    // 检查光源是否可见（复用deferred_lighting的函数）
    if (!IsLightVisible(lightIndex, brdfInput.worldPosition, brdfInput.normal)) {
        return vec3(0.0);
    }
    // 准备光源输入
    BRDFLightInput lightInput = prepareLightInput(lightIndex, brdfInput);
    
    // 如果完全在阴影中，跳过BRDF计算
    if (lightInput.visibility <= 0.0) {
        return vec3(0.0);
    }
    
    // 使用BRDF分发器计算光照（完全复用deferred_lighting）
    BRDFResult result = dispatchBRDF(brdfInput, lightInput);
    
    // 应用阴影到光照结果
    return (result.diffuse + result.specular) * lightInput.visibility;
}

/**
 * @brief 计算环境光照贡献（复用deferred_lighting的逻辑）
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
    // 采样基础颜色
    vec4 baseColor = sampleBaseColor(fs_in.texCoord);
    float alpha = baseColor.a;
    
    // 准备BRDF输入参数
    BRDFInput brdfInput = prepareBRDFInput();
    
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
    
    // 应用色调映射（与deferred_lighting保持一致）
    finalColor = finalColor / (finalColor + vec3(1.0));
    
    // 输出预乘Alpha的颜色
    // 关键修改：颜色值乘以alpha，实现预乘Alpha
    o_ForwardColor = vec4(finalColor * alpha, alpha);
}
