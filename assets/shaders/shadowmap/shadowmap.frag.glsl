// ShadowMap片段着色器 - 生成深度值到阴影贴图
#version 460 core

#include "../common/common.glsl"
#include "../common/uniforms.glsl"

// 顶点着色器输出
in VS_OUT {
    layout(location = 0) vec3 worldPos;         // 世界空间位置
    layout(location = 1) vec3 normal;           // 世界空间法线
    layout(location = 2) vec2 texCoord;         // 纹理坐标
} fs_in;

// 阴影贴图输出（仅深度）
layout(location = 0) out float o_Depth;

void main()
{
    // Alpha测试（如果材质支持）
    if (u_Material.renderProperties.z == ALPHA_MODE_MASK) {
        vec4 baseColor = u_Material.baseColor;
        if (u_Material.textureCNMROFlags.x > 0.5) {
            vec2 baseColorTexCoord = fs_in.texCoord * u_Material.baseColorTexParams.xy + u_Material.baseColorTexParams.zw;
            baseColor = texture(u_BaseColorTexture, baseColorTexCoord);
        }
        
        if (baseColor.a < u_Material.renderProperties.x) {
            discard;
        }
    }
    
    // 阴影偏移处理
    float shadowBias = u_Shadow.shadowParams.x;
    float normalBias = u_Shadow.shadowParams.y;
    
    vec3 biasedWorldPos = fs_in.worldPos;
    if (normalBias > 0.0) {
        biasedWorldPos += fs_in.normal * normalBias;
    }
    
    // 获取渲染上下文
    int shadowMapType = u_ShadowContext.shadowRenderContext.w;
    int lightIndex = u_ShadowContext.shadowRenderContext.x;
    
    // 深度值计算
    if (shadowMapType == 0) {
        // 方向光源：使用线性深度
        int cascadeIndex = u_ShadowContext.shadowRenderContext.y;
        int matrixIndex = lightIndex * MAX_CASCADES + cascadeIndex;
        vec4 shadowPos = u_Shadow.directionalMatrices[matrixIndex] * vec4(biasedWorldPos, 1.0);
        
        vec3 projCoords = shadowPos.xyz / shadowPos.w;
        projCoords = projCoords * 0.5 + 0.5;
        o_Depth = projCoords.z;
    }
    else if (shadowMapType == 1) {
        // 点光源：使用到光源的距离
        vec3 lightPos = u_Lights.lights[lightIndex].position;
        float distanceToLight = length(biasedWorldPos - lightPos);
        
        float lightRange = GetLightRange(lightIndex);
        o_Depth = distanceToLight / lightRange;
    }
    else if (shadowMapType == 2) {
        // 聚光灯：使用投影深度
        vec4 shadowPos = u_Shadow.spotLightMatrices[lightIndex] * vec4(biasedWorldPos, 1.0);
        
        vec3 projCoords = shadowPos.xyz / shadowPos.w;
        projCoords = projCoords * 0.5 + 0.5;
        o_Depth = projCoords.z;
    }
    else {
        o_Depth = gl_FragCoord.z;
    }
    
    o_Depth += shadowBias;
    o_Depth = clamp(o_Depth, 0.0, 1.0);
}
