#version 430 core
in vec2 v_TexCoord;        // 来自顶点着色器的纹理坐标

layout(std140, binding = 1) uniform MaterialUBO {
    vec4 u_BaseColor;
} Material;

out vec4 FragColor;         // 最终输出颜色

void main() {
    FragColor = vec4(Material.u_BaseColor) ; 
}