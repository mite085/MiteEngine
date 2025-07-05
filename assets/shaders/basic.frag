#version 330 core
in vec2 v_TexCoord;        // 来自顶点着色器的纹理坐标

uniform sampler2D u_Texture; // 纹理采样器
uniform vec3 u_Color;       // 基础颜色（可混合纹理）

out vec4 FragColor;         // 最终输出颜色

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = vec4(u_Color, 1.0) * texColor; // 颜色与纹理混合
}