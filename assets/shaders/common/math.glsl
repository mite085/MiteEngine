// 数学工具函数
// 线性深度转换
float LinearizeDepth(float depth, float near, float far) {
    return (2.0 * near) / (far + near - depth * (far - near));
}
// 法线编码解码
vec3 EncodeNormal(vec3 normal) {
    return normalize(normal) * 0.5 + 0.5;
}
vec3 DecodeNormal(vec3 encoded) {
    return normalize(encoded * 2.0 - 1.0);
}
// 色调映射辅助函数
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}