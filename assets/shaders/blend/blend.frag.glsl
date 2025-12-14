#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec2 texCoord;          // 纹理坐标
} fs_in;

// 最终输出颜色
layout(location = 0) out vec4 o_FinalColor;

#include "../common/common.glsl"
#include "../common/uniforms.glsl"


void main()
{
    // 采样Deferred Lighting结果（不透明物体）
    vec4 deferredColor = texture(u_LightingCombined, fs_in.texCoord);
    
    // 采样Forward半透明结果（预乘Alpha格式）
    vec4 forwardColor = texture(u_ForwardTransparent, fs_in.texCoord);
    
    // 预乘Alpha混合公式：C_final = C_forward + C_deferred * (1 - alpha)
    // 对于无效像素：alpha=0, C_forward=0，结果=C_deferred
    // 对于有效像素：正常混合
    vec3 blendedColor = forwardColor.rgb + deferredColor.rgb * (1.0 - forwardColor.a);
    
    // 输出最终颜色（Alpha始终为1.0，因为混合后不透明）
    o_FinalColor = vec4(blendedColor, 1.0);
}

// void main()
// {
//     === 临时调试：将屏幕分成12区域显示点光源ShadowMap立方体贴图的6个面 ===
//     vec2 texCoord = fs_in.texCoord;

//     // 将屏幕分成3行4列（12个区域）
//     vec2 gridCoord = texCoord * vec2(4.0, 3.0);  // 扩展到网格坐标
//     ivec2 cell = ivec2(floor(gridCoord));        // 计算单元格索引
//     vec2 localCoord = fract(gridCoord);          // 单元格内局部坐标
    
//     // 初始化采样方向
//     vec3 sampleDir = vec3(0.0);
//     int faceLayer = 0;  // 默认使用第一组立方体贴图
    
//     // 根据单元格位置设置不同的采样方向（对应立方体贴图的6个面）
//     if (cell.y == 0) {  // 第一行：+Z面（顶部）
//         if (cell.x == 1) {  // 第一行第二列
//             // +Z面：从[0,1]映射到[-1,1]
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, 1.0 - localCoord.y * 2.0, 1.0);
//         }
//     } 
//     else if (cell.y == 1) {  // 第二行：+X, +Y, -X, -Y四个面
//         if (cell.x == 0) {      // 第二行第一列：+X面（右）
//             sampleDir = vec3(1.0, 1.0 - localCoord.y * 2.0, localCoord.x * 2.0 - 1.0);
//         }
//         else if (cell.x == 1) { // 第二行第二列：+Y面（前）
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, 1.0, 1.0 - localCoord.y * 2.0);
//         }
//         else if (cell.x == 2) { // 第二行第三列：-X面（左）
//             sampleDir = vec3(-1.0, 1.0 - localCoord.y * 2.0, 1.0 - localCoord.x * 2.0);
//         }
//         else if (cell.x == 3) { // 第二行第四列：-Y面（后）
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, -1.0, localCoord.y * 2.0 - 1.0);
//         }
//     }
//     else if (cell.y == 2) {  // 第三行：-Z面（底部）
//         if (cell.x == 1) {  // 第三行第二列
//             // -Z面：从[0,1]映射到[-1,1]
//             sampleDir = vec3(localCoord.x * 2.0 - 1.0, localCoord.y * 2.0 - 1.0, -1.0);
//         }
//     }
    
//     vec3 debugColor = vec3(0.0);
//     // 在空白区域显示黑色
//     if (sampleDir == vec3(0.0)) {
//         debugColor = vec3(0.0);
//     }else{
//         // 标准化方向向量
//         sampleDir = normalize(sampleDir);
    
//         // 采样深度值
//         float depthValue = 0.0;
//         if (sampleDir != vec3(0.0)) {  // 只在有效区域内采样
//             depthValue = texture(u_ShadowMapPoint, vec4(sampleDir, faceLayer)).r;
//         }
    
//         // 可视化深度值
//         // 添加深度值范围检查
//         if (depthValue == 0.0) {
//             // 深度为0可能表示没有写入
//             debugColor = vec3(0.0, 1.0, 0.0); // 绿色表示深度为0
//         } else if (depthValue == 1.0) {
//             // 深度为1可能表示远平面
//             debugColor = vec3(0.0, 0.0, 1.0); // 蓝色表示深度为1
//         } else {
//             // 正常深度范围
//             debugColor = vec3(depthValue);
//         }
    
//     }
    
//     o_FinalColor = vec4(debugColor, 1.0);
//     return;
//     === 临时调试结束 ===
// }


// void main()
// {
//     // === 临时调试：显示方向光阴影贴图的级联（简洁两行布局）===
//     vec2 texCoord = fs_in.texCoord;
//     // 获取级联数量
//     int cascadeCount = u_Shadow.shadowConfig.w;
//     // 如果没有级联，显示黑色
//     if (cascadeCount == 0) {
//         o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
//         return;
//     }
//     // 两行布局：第一行显示级联0和1，第二行显示级联2和3
//     int maxCascadesToShow = min(cascadeCount, 4);
//     int colsPerRow = 2;
//     int rows = 2;
//     // 将屏幕分成2x2网格
//     vec2 gridCoord = texCoord * vec2(float(colsPerRow), float(rows));
//     ivec2 cell = ivec2(floor(gridCoord));
//     vec2 localCoord = fract(gridCoord);
//     // 计算级联索引
//     int cascadeIndex = cell.y * colsPerRow + cell.x;
//     // 检查是否有效
//     if (cascadeIndex >= maxCascadesToShow) {
//         o_FinalColor = vec4(0.0, 0.0, 0.0, 1.0);
//         return;
//     }
//     // 使用第一个方向光
//     int shadowMapIndex = u_Shadow.directionalShadowIndices[0].x + cascadeIndex;
//     // 采样深度值
//     float depthValue = texture(u_ShadowMapDirectional, vec3(localCoord, shadowMapIndex)).r;
//     // 简单灰度显示
//     vec3 debugColor = vec3(depthValue);
//     // 添加边框
//     float border = 0.01;
//     if (localCoord.x < border || localCoord.x > 1.0 - border || 
//         localCoord.y < border || localCoord.y > 1.0 - border) {
//         debugColor = vec3(0.0); // 黑色边框
//     }
//     // 显示级联标签
//     if (localCoord.x < 0.1 && localCoord.y > 0.9) {
//         // 级联标签颜色
//         vec3 labelColor = vec3(1.0, 1.0, 1.0);
//         if (cascadeIndex == 0) labelColor = vec3(1.0, 0.3, 0.3);
//         else if (cascadeIndex == 1) labelColor = vec3(0.3, 1.0, 0.3);
//         else if (cascadeIndex == 2) labelColor = vec3(0.3, 0.3, 1.0);
//         else if (cascadeIndex == 3) labelColor = vec3(1.0, 1.0, 0.3);
    
//         debugColor = labelColor;
//     }
//     o_FinalColor = vec4(debugColor, 1.0);
//     return;
//     // === 临时调试结束 ===
// }
