// ShadowMap顶点着色器 - 生成阴影贴图的顶点变换
#version 460 core

// 顶点输入属性
layout(location = 0) in vec3 a_Position;        // 顶点位置
layout(location = 1) in vec3 a_Normal;          // 顶点法线
layout(location = 2) in vec2 a_TexCoord;        // 纹理坐标
layout(location = 3) in vec3 a_Tangent;         // 顶点切线
layout(location = 4) in vec3 a_Bitangent;       // 顶点副切线

#include "../common/common.glsl"
#include "../common/uniforms.glsl"

// 输出到片段着色器
out VS_OUT {
    layout(location = 0) vec3 worldPos;         // 世界空间位置
    layout(location = 1) vec3 normal;           // 世界空间法线
    layout(location = 2) vec2 texCoord;         // 纹理坐标
} vs_out;

void main()
{
    // 计算世界空间位置
    vec4 worldPosition = u_Model.model * vec4(a_Position, 1.0);
    vs_out.worldPos = worldPosition.xyz;
    
    // 变换法线到世界空间
    mat3 normalMatrix = mat3(transpose(inverse(u_Model.model)));
    vs_out.normal = normalize(normalMatrix * a_Normal);
    
    // 传递纹理坐标
    vs_out.texCoord = a_TexCoord;
    
    // 根据光源类型选择正确的变换矩阵
    int shadowMapType = u_ShadowContext.shadowRenderContext.w;
    int lightIndex = u_ShadowContext.shadowRenderContext.x;
    
    if (shadowMapType == 0) {
        // 方向光源：使用级联阴影矩阵
        int cascadeIndex = u_ShadowContext.shadowRenderContext.y;
        int matrixIndex = lightIndex * MAX_CASCADES + cascadeIndex;
        gl_Position = u_Shadow.directionalMatrices[matrixIndex] * worldPosition;
    }
    else if (shadowMapType == 1) {
        // 点光源：使用立方体贴图特定面的矩阵
        int faceIndex = u_ShadowContext.shadowRenderContext.z;
        int matrixIndex = lightIndex * 6 + faceIndex;
        gl_Position = u_Shadow.pointLightMatrices[matrixIndex] * worldPosition;
    }
    else if (shadowMapType == 2) {
        // 聚光灯：使用聚光灯阴影矩阵
        gl_Position = u_Shadow.spotLightMatrices[lightIndex] * worldPosition;
    }
    else {
        // 默认使用模型视图投影矩阵
        gl_Position = u_Camera.viewProjection * worldPosition;
    }
}
