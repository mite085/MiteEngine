// 前向渲染片段着色器 - 用于透明物体和特殊渲染
#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    vec3 worldPos;          // 世界空间位置
    vec3 normal;            // 世界空间法线
    vec3 tangent;           // 世界空间切线
    vec3 bitangent;         // 世界空间副切线
    vec2 texCoord;          // 纹理坐标
    vec4 clipPos;           // 裁剪空间位置
} fs_in;

// 最终输出颜色
layout(location = 0) out vec4 o_FinalColor;

// 包含必要的头文件
#include "../common/common.glsl"
#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"
#include "../common/math.glsl"
#include "../lighting/light_calculation.glsl"
#include "../brdf/pbr.brdf.glsl"

// 纹理采样器
uniform sampler2D u_BaseColorTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetallicRoughnessTexture;
uniform sampler2D u_EmissiveTexture;
uniform sampler2D u_OcclusionTexture;

// 环境光照参数
uniform samplerCube u_IrradianceMap;            // 漫反射环境贴图
uniform samplerCube u_PrefilterMap;             // 预滤波环境贴图
uniform sampler2D u_BRDFLUT;                    // BRDF积分查找表
uniform float u_AmbientIntensity = 0.03;        // 环境光强度

// 渲染参数
uniform int u_ReceiveShadows = 1;               // 是否接收阴影
uniform int u_AlphaMode = ALPHA_MODE_OPAQUE;    // Alpha模式
uniform float u_AlphaCutoff = 0.5;              // Alpha裁剪阈值

// 调试参数
uniform int u_DebugMode = 0;                    // 调试模式

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
    
    // 双面渲染处理
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
        
        // GLTF标准：金属度在B通道，粗糙度在G通道
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
    BRDFInput input;
    
    // 基础几何信息
    input.worldPosition = fs_in.worldPos;
    input.normal = calculateNormal(fs_in.texCoord);
    input.viewDirection = normalize(u_Camera.cameraPosition - fs_in.worldPos);
    
    // 采样材质属性
    vec4 baseColor = sampleBaseColor(fs_in.texCoord);
    input.baseColor = baseColor.rgb;
    
    vec2 metallicRoughness = sampleMetallicRoughness(fs_in.texCoord);
    input.metallic = metallicRoughness.r;
    input.roughness = metallicRoughness.g;
    
    input.occlusion = sampleOcclusion(fs_in.texCoord);
    input.emission = sampleEmission(fs_in.texCoord);
    
    // 材质类型（前向渲染主要使用PBR）
    input.materialType = MATERIAL_TYPE_PBR;
    
    return input;
}

/**
 * @brief 准备环境光照输入
 * @param brdfInput BRDF输入参数
 * @return 环境光照输入
 */
BRDFAmbientInput prepareAmbientInput(BRDFInput brdfInput)
{
    BRDFAmbientInput ambientInput;
    
    // 标准化法线和视线方向
    vec3 N = normalize(brdfInput.normal);
    vec3 V = normalize(brdfInput.viewDirection);
    vec3 R = reflect(-V, N);
    
    // 采样环境贴图
    ambientInput.irradiance = texture(u_IrradianceMap, N).rgb;
    
    // 预滤波环境光
    float roughness = brdfInput.roughness;
    const float MAX_REFLECTION_LOD = 4.0;
    ambientInput.prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    
    // BRDF积分查找表
    float NdotV = max(dot(N, V), 0.0);
    ambientInput.brdfLUT = texture(u_BRDFLUT, vec2(NdotV, roughness)).rgb;
    
    // 环境光参数
    ambientInput.ambientIntensity = u_AmbientIntensity;
    
    return ambientInput;
}

/**
 * @brief 计算单个光源的贡献
 * @param lightIndex 光源索引
 * @param brdfInput BRDF输入参数
 * @return 光照贡献
 */
vec3 calculateLightContribution(uint lightIndex, BRDFInput brdfInput)
{
    // 准备光源输入
    BRDFLightInput lightInput;
    
    GPULightData light = u_Lights.lights[lightIndex];
    lightInput.lightColor = CalculateLightColor(lightIndex);
    lightInput.lightDirection = CalculateLightDirection(lightIndex, brdfInput.worldPosition);
    lightInput.attenuation = CalculateLightAttenuation(lightIndex, brdfInput.worldPosition, brdfInput.normal);
    lightInput.visibility = 1.0; // 前向渲染暂时不考虑阴影
    lightInput.lightType = uint(light.type);
    lightInput.lightPosition = light.position;
    lightInput.lightRange = GetLightRange(lightIndex);
    
    // 检查光源是否可见
    if (!IsLightVisible(lightIndex, brdfInput.worldPosition, brdfInput.normal)) {
        return vec3(0.0);
    }
    
    // 计算PBR光照
    BRDFResult result = calculateDirectBRDF(brdfInput, lightInput);
    
    return result.diffuse + result.specular;
}

/**
 * @brief Alpha测试
 * @param alpha 透明度值
 */
void performAlphaTest(float alpha)
{
    if (u_AlphaMode == ALPHA_MODE_MASK && alpha < u_AlphaCutoff) {
        discard;
    }
}

/**
 * @brief 调试显示函数
 * @param brdfInput BRDF输入参数
 * @return 调试颜色
 */
vec3 debugDisplay(BRDFInput brdfInput)
{
    switch (u_DebugMode) {
        case 1: // 显示世界位置
            return brdfInput.worldPosition * 0.1 + 0.5;
            
        case 2: // 显示法线
            return brdfInput.normal * 0.5 + 0.5;
            
        case 3: // 显示基础颜色
            return brdfInput.baseColor;
            
        case 4: // 显示金属度
            return vec3(brdfInput.metallic);
            
        case 5: // 显示粗糙度
            return vec3(brdfInput.roughness);
            
        case 6: // 显示AO
            return vec3(brdfInput.occlusion);
            
        case 7: // 显示自发光
            return brdfInput.emission;
            
        case 8: // 显示切线
            return fs_in.tangent * 0.5 + 0.5;
            
        case 9: // 显示纹理坐标
            return vec3(fs_in.texCoord, 0.0);
            
        default:
            return vec3(0.0);
    }
}

void main()
{
    // 准备BRDF输入参数
    BRDFInput brdfInput = prepareBRDFInput();
    
    // Alpha测试
    vec4 baseColor = sampleBaseColor(fs_in.texCoord);
    performAlphaTest(baseColor.a);
    
    // 调试模式显示
    if (u_DebugMode > 0) {
        vec3 debugColor = debugDisplay(brdfInput);
        o_FinalColor = vec4(debugColor, baseColor.a);
        return;
    }
    
    // 初始化最终颜色
    vec3 finalColor = vec3(0.0);
    
    // 计算环境光照
    BRDFAmbientInput ambientInput = prepareAmbientInput(brdfInput);
    BRDFResult ambientResult = calculateAmbientBRDF(brdfInput, ambientInput);
    finalColor += ambientResult.diffuse + ambientResult.specular;
    
    // 计算所有直接光源贡献
    for (uint i = 0u; i < u_Lights.header.lightCount; i++) {
        vec3 lightContribution = calculateLightContribution(i, brdfInput);
        finalColor += lightContribution;
    }
    
    // 添加自发光
    finalColor += brdfInput.emission;
    
    // 应用色调映射（简单Reinhard）
    finalColor = finalColor / (finalColor + vec3(1.0));
    
    // 伽马校正
    finalColor = pow(finalColor, vec3(1.0/2.2));
    
    // 输出最终颜色（支持透明度）
    o_FinalColor = vec4(finalColor, baseColor.a);
}
