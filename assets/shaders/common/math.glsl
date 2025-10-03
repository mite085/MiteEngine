// 数学工具函数
#ifndef MATH_GLSL
#define MATH_GLSL

// 将法线从[0,1]范围转换到[-1,1]范围
vec3 unpackNormal(vec3 packedNormal)
{
    return normalize(packedNormal * 2.0 - 1.0);
}

// 将法线从[-1,1]范围转换到[0,1]范围
vec3 packNormal(vec3 normal)
{
    return normal * 0.5 + 0.5;
}

// 线性深度计算
float linearizeDepth(float depth, float near, float far)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}

// 计算TBN矩阵
mat3 calculateTBNMatrix(vec3 normal, vec3 tangent, vec3 bitangent)
{
    return mat3(tangent, bitangent, normal);
}

#endif
