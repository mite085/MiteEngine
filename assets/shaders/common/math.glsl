// 数学工具函数
#ifndef MATH_GLSL
#define MATH_GLSL

// 常量定义
const float PI = 3.14159265359;
const float EPSILON = 0.00001;

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

// 计算反射向量
vec3 calculateReflection(vec3 incident, vec3 normal)
{
    return reflect(-incident, normal);
}

// 计算半角向量
vec3 calculateHalfVector(vec3 lightDir, vec3 viewDir)
{
    return normalize(lightDir + viewDir);
}

// 计算菲涅尔效应（Schlick近似）
vec3 calculateFresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 计算菲涅尔效应（带粗糙度）
vec3 calculateFresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 计算法线分布函数（GGX/Trowbridge-Reitz）
float calculateDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

// 计算几何函数（Smith方法）
float calculateGeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
float calculateGeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = calculateGeometrySchlickGGX(NdotV, roughness);
    float ggx1 = calculateGeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// 将角度转换为弧度
float radians(float degrees)
{
    return degrees * PI / 180.0;
}

// 将弧度转换为角度  
float degrees(float radians)
{
    return radians * 180.0 / PI;
}

#endif
