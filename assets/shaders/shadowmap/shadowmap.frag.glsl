// ShadowMap片段着色器 - 生成深度值到阴影贴图
#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec4 worldPosition;          // 世界坐标
} fs_in;

#include "../common/uniforms.glsl"
#include "../common/lights_ssbo.glsl"

void main()
{
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
        // gl_FragDepth = gl_FragCoord.z;
    }
    else if (shadowMapType == 2) {
        // 聚光灯：使用默认的非线性深度（保持原有逻辑）
        // gl_FragDepth = gl_FragCoord.z;
    }
}
