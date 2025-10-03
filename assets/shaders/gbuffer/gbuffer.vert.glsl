// GBuffer顶点着色器 - 生成GBuffer纹理的顶点坐标
#version 460 core

layout(location = 0) in vec3 a_Position;        // 顶点位置
layout(location = 1) in vec3 a_Normal;          // 顶点法线
layout(location = 2) in vec2 a_TexCoord;        // 纹理坐标
layout(location = 3) in vec3 a_Tangent;         // 顶点切线
layout(location = 4) in vec3 a_Bitangent;       // 顶点副切线

#include "../common/uniforms.glsl"

// 模型矩阵 - 用于将顶点从模型空间变换到世界空间
uniform mat4 u_Model;

// 输出到片段着色器
out VS_OUT {
    vec3 worldPos;          // 世界空间位置
    vec3 normal;            // 世界空间法线
    vec3 tangent;           // 世界空间切线
    vec3 bitangent;         // 世界空间副切线
    vec2 texCoord;          // 纹理坐标
} vs_out;

void main()
{
    // 计算世界空间位置
    // 使用模型矩阵将顶点从模型空间变换到世界空间
    vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
    vs_out.worldPos = worldPosition.xyz;
    
    // 构建模型矩阵的3x3部分用于法线变换（去除缩放和位移）
    // 注意：对于非均匀缩放，需要使用法线矩阵来正确变换法线
    mat3 normalMatrix = mat3(transpose(inverse(u_Model)));
    
    // 变换法线到世界空间
    // 法线需要特殊处理，确保在非均匀缩放下仍保持正确方向
    vs_out.normal = normalize(normalMatrix * a_Normal);
    
    // 变换切线和副切线到世界空间
    // 切线和副切线使用与顶点相同的变换方式
    vs_out.tangent = normalize(normalMatrix * a_Tangent);
    vs_out.bitangent = normalize(normalMatrix * a_Bitangent);
    
    // 传递纹理坐标（不需要变换）
    vs_out.texCoord = a_TexCoord;
    
    // 计算裁剪空间位置
    // 组合世界坐标和相机视图投影矩阵得到最终的裁剪空间坐标
    gl_Position = u_Camera.viewProjection * worldPosition;
}
