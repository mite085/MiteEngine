// ShadowMap片段着色器 - 生成深度值到阴影贴图
#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec4 worldPosition;          // 世界坐标
    layout(location = 1) vec2 texCoord;         // 纹理坐标
} fs_in;

#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"


// Alpha Test函数 - 返回true表示通过，false表示丢弃
bool performAlphaTest() {
    // 获取Alpha模式
    uint alphaMode = uint(u_Material.renderProperties.z);
    
    // OPAQUE模式：总是通过
    if (alphaMode == ALPHA_MODE_OPAQUE) {
        return true;
    }
    
    // BLEND模式：完全丢弃（返回false）
    if (alphaMode == ALPHA_MODE_BLEND) {
        return false;
    }
    
    // MASK模式：进行Alpha Test
    if (alphaMode == ALPHA_MODE_MASK) {
        // 采样基础色
        vec4 baseColor = u_Material.baseColor;
        
        // 如果有纹理，采样纹理
        if (u_Material.textureCNMROFlags.x > 0.5) {
            vec2 uv = fs_in.texCoord * u_Material.baseColorTexParams.xy 
                    + u_Material.baseColorTexParams.zw;
            baseColor *= texture(u_BaseColorTexture, uv);
        }
        
        // Alpha Test：小于阈值则丢弃
        float alphaCutoff = u_Material.renderProperties.x;
        return baseColor.a >= alphaCutoff;
    }
    
    // 默认通过
    return true;
}

void main()
{
    // 执行Alpha Test
    if (!performAlphaTest()) {
        discard;
    }

    // 光源类型
    int shadowMapType = u_ShadowContext.shadowRenderContext.w;
    int lightIndex = u_ShadowContext.shadowRenderContext.x;

    if (shadowMapType == 1) {
        // 点光源：计算到光源的线性距离并归一化

        // 获取光源位置坐标
        vec3 lightPos = u_Lights.lights[lightIndex].position;

        // 计算当前片段的深度（标准化到[0,1]）
        float closestDepth = length(fs_in.worldPosition.xyz - lightPos);
        float linearDepth = closestDepth / GetLightRange(lightIndex);
        
        // 存储线性深度到samplerCubeArray
        gl_FragDepth = linearDepth;
    }
    else if (shadowMapType == 0) {
        // 方向光源：使用默认的非线性深度（保持原有逻辑）
        gl_FragDepth = gl_FragCoord.z;
    }
    else if (shadowMapType == 2) {
        // 聚光灯：使用默认的非线性深度（保持原有逻辑）
        gl_FragDepth = gl_FragCoord.z;
    }
}
