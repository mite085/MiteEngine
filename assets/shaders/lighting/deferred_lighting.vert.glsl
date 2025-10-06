// 延迟光照全屏四边形顶点着色器
#version 460 core

// 全屏四边形的顶点位置和纹理坐标
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// 输出到片段着色器
out VS_OUT {
    vec2 texCoord;          // 纹理坐标
    vec3 viewRay;           // 视图射线（用于重建世界位置）
} vs_out;

// 相机参数
#include "../common/uniforms.glsl"

void main()
{
    // 传递纹理坐标
    vs_out.texCoord = a_TexCoord;
    
    // 计算视图射线（用于在片段着色器中重建世界位置）
    // 根据投影类型计算不同的视图射线
    if (u_Camera.projectionType == PROJECTION_PERSPECTIVE) {
        // 透视投影--从NDC坐标计算视图射线
        vec4 clipPos = vec4(a_Position.xy, 0.0, 1.0);
        vec4 viewPos = inverse(u_Camera.projection) * clipPos;
        vs_out.viewRay = viewPos.xyz;
    } else {
        // 正交投影--直接使用顶点位置
        vs_out.viewRay = vec3(a_Position.xy, 0.0);
    }
    
    // 输出裁剪空间位置（直接使用顶点位置作为全屏四边形）
    gl_Position = vec4(a_Position.xyz, 1.0);
}
