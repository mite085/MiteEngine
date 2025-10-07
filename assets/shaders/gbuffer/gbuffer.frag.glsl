// GBuffer片段着色器 - 填充GBuffer纹理
#version 460 core

#include "../common/common.glsl"
#include "../common/uniforms.glsl" 
#include "../common/math.glsl"

// 顶点着色器输出
in VS_OUT {
    vec3 worldPos;          // 世界空间位置
    vec3 normal;            // 世界空间法线
    vec3 tangent;           // 世界空间切线
    vec3 bitangent;         // 世界空间副切线
    vec2 texCoord;          // 纹理坐标
} fs_in;

// GBuffer输出 - 与C++端GBuffer布局对应
layout(location = GBUFFER_WORLDPOS_DEPTH) out vec4 o_WorldPosDepth;        // 世界坐标+深度
layout(location = GBUFFER_BASECOLOR_MATTYPE) out vec4 o_BaseColorMatType;  // 基础色+材质类型
layout(location = GBUFFER_METALLICROUGHNESS_AO) out vec4 o_MetallicRoughnessAO; // 金属粗糙度+AO
layout(location = GBUFFER_NORMAL_SCALE) out vec4 o_NormalScale;            // 法线+法线强度
layout(location = GBUFFEE_EMISSION_ALPHA) out vec4 o_EmissionAlpha;        // 自发光+透明度
layout(location = GBUFFER_NPR_PARAM) out vec4 o_NPRParameters;             // NPR参数
layout(location = GBUFFER_NPR_COLOR) out vec4 o_NPRColors;                 // NPR颜色

// 纹理采样器
uniform sampler2D u_BaseColorTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetallicRoughnessTexture;
uniform sampler2D u_EmissiveTexture;
uniform sampler2D u_OcclusionTexture;

void main()
{
    // =========================================================================
    // 纹理坐标变换（考虑缩放和偏移）
    // =========================================================================
    vec2 baseColorTexCoord = fs_in.texCoord * u_Material.baseColorTexParams.xy + u_Material.baseColorTexParams.zw;
    vec2 normalTexCoord = fs_in.texCoord * u_Material.normalTexParams.xy + u_Material.normalTexParams.zw;
    vec2 mrTexCoord = fs_in.texCoord * u_Material.mrTexParams.xy + u_Material.mrTexParams.zw;
    vec2 emissiveTexCoord = fs_in.texCoord * u_Material.emissiveTexParams.xy + u_Material.emissiveTexParams.zw;
    vec2 occlusionTexCoord = fs_in.texCoord * u_Material.occlusionTexParams.xy + u_Material.occlusionTexParams.zw;
    
    // =========================================================================
    // 基础颜色采样和处理
    // =========================================================================
    vec4 baseColor = u_Material.baseColor;
    if (u_Material.textureCNMROFlags.x > 0.5) {
        baseColor = texture(u_BaseColorTexture, baseColorTexCoord);
        // 如果是sRGB纹理，需要转换到线性空间
        baseColor.rgb = pow(baseColor.rgb, vec3(2.2));
    }
    
    // =========================================================================
    // Alpha测试和透明度处理
    // =========================================================================
    float alpha = baseColor.a;
    
    // Alpha裁剪模式--根据阈值丢弃片段
    if (u_Material.renderProperties.z == ALPHA_MODE_MASK) {
        if (alpha < u_Material.renderProperties.x) {
            discard;
        }
        alpha = 1.0; // 裁剪模式下，通过测试的片段视为不透明
    }
    
    // Alpha混合模式--后续在前向渲染中处理
    // 对于延迟渲染，混合物体通常需要特殊处理
    
    // =========================================================================
    // 法线计算（考虑法线贴图）
    // =========================================================================
    vec3 normal = fs_in.normal;
    
    if (u_Material.textureCNMROFlags.y > 0.5) {
        // 从法线贴图采样并解包到[-1,1]范围
        vec3 tangentNormal = unpackNormal(texture(u_NormalTexture, normalTexCoord).rgb);
        
        // 构建TBN矩阵（切线空间到世界空间的变换矩阵）
        mat3 TBN = mat3(
            normalize(fs_in.tangent),
            normalize(fs_in.bitangent), 
            normalize(fs_in.normal)
        );
        
        // 将切线空间法线变换到世界空间
        normal = normalize(TBN * tangentNormal);
        
        // 应用法线贴图强度
        if (u_Material.normalScale.x < 1.0) {
            normal = normalize(mix(fs_in.normal, normal, u_Material.normalScale.x));
        }
    }
    
    // 确保法线朝向相机（双面渲染处理）
    if (u_Material.renderProperties.y > 0.5) {
        // 双面渲染--根据视线方向调整法线
        vec3 viewDir = normalize(u_Camera.cameraPosition - fs_in.worldPos);
        if (dot(normal, viewDir) < 0.0) {
            normal = -normal;
        }
    }
    
    // =========================================================================
    // PBR材质参数采样
    // =========================================================================
    
    // 金属度和粗糙度采样
    vec2 metallicRoughness = u_Material.metallicRoughnessAO.xy;
    if (u_Material.textureCNMROFlags.z > 0.5) {
        vec4 mrSample = texture(u_MetallicRoughnessTexture, mrTexCoord);
        // GLTF标准--金属度在B通道，粗糙度在G通道
        metallicRoughness.r = mrSample.b; // 金属度
        metallicRoughness.g = mrSample.g; // 粗糙度
    }
    
    // 环境光遮蔽采样
    float occlusion = u_Material.metallicRoughnessAO.z;
    if (u_Material.textureCNMROFlags.w > 0.5) {
        occlusion = texture(u_OcclusionTexture, occlusionTexCoord).r;
    }
    
    // =========================================================================
    // 自发光采样
    // =========================================================================
    vec3 emission = u_Material.emission.rgb * u_Material.emission.a;
    if (u_Material.textureEmissionFlag.x > 0.5) {
        vec3 emissiveSample = texture(u_EmissiveTexture, emissiveTexCoord).rgb;
        // 自发光纹理通常需要HDR处理
        emission = emissiveSample * u_Material.emission.a;
        emission = pow(emission, vec3(2.2)); // sRGB到线性
    }
    
    // =========================================================================
    // 深度计算
    // =========================================================================
    // 计算线性深度用于后续光照计算
    float linearDepth = linearizeDepth(gl_FragCoord.z, u_Camera.nearPlane, u_Camera.farPlane);
    
    // =========================================================================
    // 材质类型判断
    // =========================================================================
    uint materialType = u_Material.materialInfo.x;
    
    // =========================================================================
    // 输出到GBuffer
    // =========================================================================
    
    // GBuffer0: 世界坐标(XYZ) + 线性深度(W)
    o_WorldPosDepth = vec4(fs_in.worldPos, linearDepth);
    
    // GBuffer1: 基础色(RGB) + 材质类型(A)
    o_BaseColorMatType = vec4(baseColor.rgb, float(materialType));
    
    // GBuffer2: 金属度(R) + 粗糙度(G) + AO(B) + 保留位(A)
    o_MetallicRoughnessAO = vec4(metallicRoughness.r, metallicRoughness.g, occlusion, 0.0);
    
    // GBuffer3: 世界空间法线(XYZ) + 法线贴图强度(W)
    o_NormalScale = vec4(packNormal(normal), u_Material.normalScale.x);
    
    // GBuffer4: 自发光颜色(RGB) + 透明度(A)
    o_EmissionAlpha = vec4(emission, alpha);
    
    // GBuffer5: NPR参数
    // x:色阶阈值, y:色阶平滑度, z:高光尺寸, w:描边宽度
    o_NPRParameters = u_Material.nprParameters;
    
    // GBuffer6: NPR颜色
    // xyz:阴影色调, w:边缘光强度
    o_NPRColors = u_Material.nprColors;
    
}
