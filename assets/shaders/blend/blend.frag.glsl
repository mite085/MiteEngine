#version 460 core

// 输入从顶点着色器传递的数据
in VS_OUT {
    layout(location = 0) vec2 texCoord;          // 纹理坐标
} fs_in;

// 最终输出颜色
layout(location = 0) out vec4 o_FinalColor;

// 输入纹理
layout(binding = 14) uniform sampler2D u_LightingCombined;      // Deferred Lighting结果
layout(binding = 24) uniform sampler2D u_ForwardTransparent;    // Forward半透明结果

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
