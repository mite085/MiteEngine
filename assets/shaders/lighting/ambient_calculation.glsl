// 环境光照计算辅助函数
#ifndef AMBIENT_CALCULATION_GLSL
#define AMBIENT_CALCULATION_GLSL

#include "../brdf/brdf_common.glsl"

// 解析近似BRDF LUT（替代纹理查找）
// 使用Schlick近似计算菲涅尔项和几何项的组合
vec3 approximateBRDFLUT(float NdotV, float roughness, vec3 F0) {
    // Schlick菲涅尔近似：F0为基础反射率，随视角变化
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
    
    // 简化的几何遮蔽项近似，基于粗糙度
    float G = 1.0 / (NdotV * (1.0 - roughness * 0.5) + roughness * 0.5);
    
    return F * G;  // 返回近似的BRDF积分结果
}

// 将3D方向向量转换为球面坐标（UV坐标）
// 用于在2D环境贴图中查找对应方向的环境光
vec2 directionToSpherical(vec3 dir) {
    // 计算方位角（水平角度），映射到[0,1]
    float phi = atan(dir.z, dir.x);
    // 计算极角（垂直角度），映射到[0,1]
    float theta = acos(dir.y);
    return vec2(phi / (2.0 * PI) + 0.5, theta / PI);
}

// Hammersley序列生成器
// 生成低差异序列，使采样点更均匀分布
vec2 hammersley(int i, int N) {
    uint bits = uint(i);
    // 位反转操作，生成均匀分布
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    // 返回[0,1]范围内的均匀分布点
    return vec2(float(i) / float(N), float(bits) * 2.3283064365386963e-10);
}

// 余弦权重重要性采样
// 生成偏向法线方向的采样点，适合漫反射计算
vec3 importanceSampleCosine(vec2 xi, vec3 N) {
    // 在圆盘上均匀采样半径
    float r = sqrt(xi.x);
    // 均匀采样角度
    float theta = 2.0 * PI * xi.y;
    
    // 在局部坐标系中生成采样点（余弦分布）
    vec3 localSample = vec3(r * cos(theta), sqrt(1.0 - xi.x), r * sin(theta));
    
    // 构建切线空间基向量
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    // 将局部采样点转换到世界坐标系
    return tangent * localSample.x + bitangent * localSample.y + N * localSample.z;
}

// GGX分布重要性采样
// 生成基于粗糙度的镜面反射采样点，粗糙度越高采样越分散
vec3 importanceSampleGGX(vec2 xi, vec3 N, float roughness) {
    float a = roughness * roughness;  // 将感知粗糙度转换为物理粗糙度
    
    // 基于GGX NDF生成采样方向
    float phi = 2.0 * PI * xi.x;  // 方位角均匀分布
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));  // 极角基于GGX分布
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    // 在局部坐标系中生成采样点
    vec3 localSample = vec3(sinTheta * cos(phi), cosTheta, sinTheta * sin(phi));
    
    // 构建切线空间基向量
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    // 将局部采样点转换到世界坐标系
    return tangent * localSample.x + bitangent * localSample.y + N * localSample.z;
}

// 近似计算辐照度（漫反射环境光）
// 通过对半球方向进行重要性采样并加权平均得到
vec3 approximateIrradiance(vec3 N, sampler2D hdrEnvMap) {
    vec3 irradiance = vec3(0.0);
    const int samples = 32;  // 采样数量，影响质量和性能
    
    for(int i = 0; i < samples; i++) {
        // 使用Hammersley序列生成均匀分布的采样点
        vec2 xi = hammersley(i, samples);
        // 基于余弦权重的重要性采样，更重视法线方向附近的样本
        vec3 L = importanceSampleCosine(xi, N);
        
        // 将采样方向转换为UV坐标
        vec2 uv = directionToSpherical(L);
        // 从环境贴图采样颜色
        vec3 sampleColor = texture(hdrEnvMap, uv).rgb;
        // 使用余弦权重累加，符合Lambertian漫反射模型
        irradiance += sampleColor * max(dot(N, L), 0.0);
    }
    
    // 返回平均辐照度
    return irradiance / float(samples);
}

// 近似计算预滤波环境光（镜面反射）
// 基于粗糙度对反射方向周围进行模糊采样
vec3 approximatePrefilteredColor(vec3 R, float roughness, sampler2D hdrEnvMap) {
    vec3 prefiltered = vec3(0.0);
    float totalWeight = 0.0;
    const int samples = 16;  // 镜面反射采样数通常少于漫反射
    
    for(int i = 0; i < samples; i++) {
        vec2 xi = hammersley(i, samples);
        // 使用GGX分布的重要性采样，粗糙度影响采样分布
        vec3 H = importanceSampleGGX(xi, R, roughness);
        // 根据半角向量计算光线方向
        vec3 L = normalize(2.0 * dot(R, H) * H - R);
        
        float NdotL = max(dot(R, L), 0.0);
        if(NdotL > 0.0) {
            vec2 uv = directionToSpherical(L);
            vec3 sampleColor = texture(hdrEnvMap, uv).rgb;
            
            // 基于粗糙度和角度的权重
            float weight = NdotL;
            prefiltered += sampleColor * weight;
            totalWeight += weight;
        }
    }
    
    // 返回加权平均的预滤波颜色
    return prefiltered / max(totalWeight, 0.001);  // 避免除零
}

/**
 * @brief 准备环境光照输入参数
 * @param brdfInput BRDF输入参数
 * @param hdrEnvMap 环境纹理
 * @return 环境光照输入
 */
BRDFAmbientInput prepareAmbientInputApprox(BRDFInput brdfInput, sampler2D hdrEnvMap) {
    BRDFAmbientInput ambientInput;
    
    // 标准化法线和视线方向
    vec3 N = normalize(brdfInput.normal);
    vec3 V = normalize(brdfInput.viewDirection);
    vec3 R = reflect(-V, N);  // 计算反射方向
    
    // 计算F0（基础反射率）：根据金属度在电介质和金属值之间插值
    vec3 F0 = mix(vec3(0.04), brdfInput.baseColor, brdfInput.metallic);

    // 近似计算漫反射环境光（辐照度）
    ambientInput.irradiance = approximateIrradiance(N, hdrEnvMap);
    
    // 近似计算预滤波环境光（镜面反射）
    float roughness = brdfInput.roughness;
    ambientInput.prefilteredColor = approximatePrefilteredColor(R, roughness, hdrEnvMap);
    
    // 使用解析近似替代BRDF LUT纹理查找
    float NdotV = max(dot(N, V), 0.0);
    ambientInput.brdfLUT = approximateBRDFLUT(NdotV, roughness, F0);
    
    // 环境光强度参数(待后续确定输入接口后启用)
    ambientInput.ambientIntensity = 1.0f;
    
    return ambientInput;
}
#endif