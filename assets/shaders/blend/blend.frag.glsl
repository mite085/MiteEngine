#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec2 texCoord;          // 纹理坐标
} fs_in;

// 最终输出颜色
layout(location = 0) out vec4 o_FinalColor;

#include "../common/common.glsl"
#include "../common/uniforms.glsl"


// void main()
// {
//     // 采样Deferred Lighting结果（不透明物体）
//     vec4 deferredColor = texture(u_LightingCombined, fs_in.texCoord);
    
//     // 采样Forward半透明结果（预乘Alpha格式）
//     vec4 forwardColor = texture(u_ForwardTransparent, fs_in.texCoord);
    
//     // 预乘Alpha混合公式：C_final = C_forward + C_deferred * (1 - alpha)
//     // 对于无效像素：alpha=0, C_forward=0，结果=C_deferred
//     // 对于有效像素：正常混合
//     vec3 blendedColor = forwardColor.rgb + deferredColor.rgb * (1.0 - forwardColor.a);
    
//     // 输出最终颜色（Alpha始终为1.0，因为混合后不透明）
//     o_FinalColor = vec4(blendedColor, 1.0);
// }

void main()
{
    // === 临时调试：显示方向光阴影贴图的级联（简洁两行布局）===
    vec2 texCoord = fs_in.texCoord;
    // 获取级联数量
    int cascadeCount = u_Shadow.shadowConfig.w;
    // 如果没有级联，显示黑色
    if (cascadeCount == 0) {
        o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    // 两行布局：第一行显示级联0和1，第二行显示级联2和3
    int maxCascadesToShow = min(cascadeCount, 4);
    int colsPerRow = 2;
    int rows = 2;
    // 将屏幕分成2x2网格
    vec2 gridCoord = texCoord * vec2(float(colsPerRow), float(rows));
    ivec2 cell = ivec2(floor(gridCoord));
    vec2 localCoord = fract(gridCoord);
    // 计算级联索引
    int cascadeIndex = cell.y * colsPerRow + cell.x;
    // 检查是否有效
    if (cascadeIndex >= maxCascadesToShow) {
        o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    // 使用第一个方向光
    int shadowMapIndex = u_Shadow.directionalShadowIndices[0].x + cascadeIndex;
    // 采样深度值
    float depthValue = texture(u_ShadowMapDirectional, vec3(localCoord, shadowMapIndex)).r;
    // 简单灰度显示
    vec3 debugColor = vec3(depthValue);
    // 添加边框
    float border = 0.01;
    if (localCoord.x < border || localCoord.x > 1.0 - border || 
        localCoord.y < border || localCoord.y > 1.0 - border) {
        debugColor = vec3(0.0); // 黑色边框
    }
    // 显示级联标签
    if (localCoord.x < 0.1 && localCoord.y > 0.9) {
        // 级联标签颜色
        vec3 labelColor = vec3(1.0, 1.0, 1.0);
        if (cascadeIndex == 0) labelColor = vec3(1.0, 0.3, 0.3);
        else if (cascadeIndex == 1) labelColor = vec3(0.3, 1.0, 0.3);
        else if (cascadeIndex == 2) labelColor = vec3(0.3, 0.3, 1.0);
        else if (cascadeIndex == 3) labelColor = vec3(1.0, 1.0, 0.3);
    
        debugColor = labelColor;
    }
    o_FinalColor = vec4(debugColor, 1.0);
    return;
    // === 临时调试结束 ===
}