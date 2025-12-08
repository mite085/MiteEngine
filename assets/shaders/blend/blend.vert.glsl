#version 460 core

// 全屏四边形的顶点位置和纹理坐标
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// 输出到片段着色器
out VS_OUT {
    layout(location = 0) vec2 texCoord;          // 纹理坐标
} vs_out;

void main()
{
    // 传递纹理坐标
    vs_out.texCoord = a_TexCoord;
    
    // 输出裁剪空间位置（直接使用顶点位置作为全屏四边形）
    gl_Position = vec4(a_Position.xyz, 1.0);
}
