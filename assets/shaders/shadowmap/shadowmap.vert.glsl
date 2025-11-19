// ShadowMap顶点着色器 - 生成阴影贴图的顶点变换
#version 460 core

// 顶点输入属性(仅关心位置)
layout(location = 0) in vec3 a_Position;        // 顶点位置

// 输出到片段着色器
out VS_OUT {
    layout(location = 0) vec4 worldPosition;    // 世界坐标
} vs_out;

#include "../common/uniforms.glsl"

void main()
{
    // 计算世界空间位置
    vs_out.worldPosition = u_Model.model * vec4(a_Position, 1.0);
    
    // 根据光源类型选择正确的变换矩阵
    int shadowMapType = u_ShadowContext.shadowRenderContext.w;
    int lightIndex = u_ShadowContext.shadowRenderContext.x;
    
    if (shadowMapType == 0) {
        // 方向光源：使用级联阴影矩阵
        int cascadeIndex = u_ShadowContext.shadowRenderContext.y;
        int matrixIndex = lightIndex * MAX_CASCADES + cascadeIndex;
        gl_Position = u_Shadow.directionalMatrices[matrixIndex] * vs_out.worldPosition;
    }
    else if (shadowMapType == 1) {
        // 点光源：使用立方体贴图特定面的矩阵
        int faceIndex = u_ShadowContext.shadowRenderContext.z;
        int matrixIndex = lightIndex * 6 + faceIndex;
        gl_Position = u_Shadow.pointLightMatrices[matrixIndex] * vs_out.worldPosition;
    }
    else if (shadowMapType == 2) {
        // 聚光灯：使用聚光灯阴影矩阵
        gl_Position = u_Shadow.spotLightMatrices[lightIndex] * vs_out.worldPosition;
    }
    else {
        // 默认使用模型视图投影矩阵
        gl_Position = u_Camera.viewProjection * vs_out.worldPosition;
    }
}
