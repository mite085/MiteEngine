#version 430 core
layout(location = 0) in vec3 aPos;      // 顶点位置
layout(location = 1) in vec2 aTexCoord; // 纹理坐标

uniform mat4 u_Model;      // 模型矩阵
uniform mat4 u_View;       // 视图矩阵
uniform mat4 u_Projection; // 投影矩阵

out vec2 v_TexCoord;       // 传递纹理坐标到片段着色器

void main() {
    v_TexCoord = aTexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}