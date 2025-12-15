// 前向渲染顶点着色器 - 用于透明物体和特殊渲染
#version 460 core

// 顶点属性
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

// 输出到片段着色器
out VS_OUT {
    layout(location = 0) vec3 worldPos;          // 世界空间位置
    layout(location = 1) vec3 normal;            // 世界空间法线
    layout(location = 2) vec3 tangent;           // 世界空间切线
    layout(location = 3) vec3 bitangent;         // 世界空间副切线
    layout(location = 4) vec2 texCoord;          // 纹理坐标
} vs_out;

// 包含必要的头文件
#include "../common/uniforms.glsl"

void main()
{
    // 计算世界空间位置
    vec4 worldPosition = u_Model.model * vec4(a_Position, 1.0);
    vs_out.worldPos = worldPosition.xyz;
    
    // 构建法线矩阵（用于正确变换法线）
    mat3 normalMatrix = mat3(transpose(inverse(u_Model.model)));
    
    // 变换法线、切线、副切线到世界空间
    vs_out.normal = normalize(normalMatrix * a_Normal);
    vs_out.tangent = normalize(normalMatrix * a_Tangent);
    vs_out.bitangent = normalize(normalMatrix * a_Bitangent);
    
    // 传递纹理坐标
    vs_out.texCoord = a_TexCoord;
    
    // 计算裁剪空间位置
    gl_Position = u_Camera.viewProjection * worldPosition;
}
