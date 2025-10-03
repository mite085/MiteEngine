// BRDF通用接口定义
#ifndef BRDF_COMMON_GLSL
#define BRDF_COMMON_GLSL

#include "../common/common.glsl"
#include "../common/math.glsl"

// BRDF输入参数结构体
struct BRDFInput {
    // 表面属性
    vec3 worldPosition;     // 世界空间位置
    vec3 normal;            // 世界空间法线
    vec3 viewDirection;     // 视线方向（世界空间）
    
    // 材质属性
    vec3 baseColor;         // 基础颜色（线性空间）
    float metallic;         // 金属度 [0.0-1.0]
    float roughness;        // 粗糙度 [0.0-1.0]
    float occlusion;        // 环境光遮蔽 [0.0-1.0]
    vec3 emission;          // 自发光颜色
    
    // 材质类型
    uint materialType;      // 材质类型标识
};

// BRDF光照输入结构体
struct BRDFLightInput {
    vec3 lightColor;        // 光源颜色（包含强度）
    vec3 lightDirection;    // 光源方向（指向光源）
    float attenuation;      // 光照衰减
    float visibility;       // 阴影可见性 [0.0-1.0]
    
    // 光源类型特定数据
    uint lightType;         // 光源类型
    vec3 lightPosition;     // 光源位置（用于点/面光源）
    float lightRange;       // 光源范围
};

// BRDF输出结果结构体
struct BRDFResult {
    vec3 diffuse;           // 漫反射贡献
    vec3 specular;          // 镜面反射贡献
    vec3 emission;          // 自发光贡献
    float alpha;            // 透明度
};

// BRDF环境光照输入
struct BRDFAmbientInput {
    vec3 irradiance;        // 漫反射环境光（IBL）
    vec3 prefilteredColor;  // 预滤波环境光（镜面反射）
    vec3 brdfLUT;           // BRDF积分查找表
    float ambientIntensity; // 环境光强度
};

// BRDF函数接口
// 所有BRDF实现都应该提供这些函数

/**
 * @brief 计算直接光照的BRDF贡献
 * @param brdfInput BRDF输入参数
 * @param lightInput 光源输入参数
 * @return BRDF光照计算结果
 */
BRDFResult calculateDirectBRDF(BRDFInput brdfInput, BRDFLightInput lightInput);

/**
 * @brief 计算环境光照的BRDF贡献（IBL）
 * @param brdfInput BRDF输入参数
 * @param ambientInput 环境光照输入
 * @return BRDF环境光照结果
 */
BRDFResult calculateAmbientBRDF(BRDFInput brdfInput, BRDFAmbientInput ambientInput);

/**
 * @brief 计算基础反射率F0
 * @param baseColor 基础颜色
 * @param metallic 金属度
 * @return 基础反射率
 */
vec3 calculateF0(vec3 baseColor, float metallic);

/**
 * @brief 验证BRDF输入参数的合法性
 * @param input BRDF输入参数
 * @return 参数是否合法
 */
bool validateBRDFInput(BRDFInput input);

/**
 * @brief 准备BRDF计算所需的中间参数
 * @param brdfInput BRDF输入参数
 * @param lightInput 光源输入参数
 * @return 准备好的中间参数
 */
struct BRDFIntermediate {
    vec3 F0;                // 基础反射率
    vec3 F;                 // 菲涅尔项
    float NdotL;            // 法线·光源方向
    float NdotV;            // 法线·视线方向
    float NdotH;            // 法线·半角向量
    float VdotH;            // 视线·半角向量
    float LdotH;            // 光源·半角向量
    vec3 H;                 // 半角向量
    float D;                // 法线分布函数
    float G;                // 几何遮蔽函数
};

BRDFIntermediate prepareBRDFIntermediate(BRDFInput brdfInput, BRDFLightInput lightInput);

#endif
