#version 430 core
in vec2 v_TexCoord;        // 来自顶点着色器的纹理坐标

uniform vec3 u_Color;       // 基础颜色

out vec4 FragColor;         // 最终输出颜色

void main() {
    FragColor = vec4(u_Color, 1.0) ; 
}