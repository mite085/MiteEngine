// 通用常量和结构体定义
#ifndef COMMON_GLSL
#define COMMON_GLSL

// 材质类型枚举
#define MATERIAL_TYPE_PBR 0.0
#define MATERIAL_TYPE_NPR 1.0

// GBuffer索引常量
const int GBUFFER_WORLDPOS_DEPTH = 0;
const int GBUFFER_BASECOLOR_MATTYPE = 1;
const int GBUFFER_METALLICROUGHNESS_AO = 2;
const int GBUFFER_NORMAL_SCALE = 3;
const int GBUFFEE_EMISSION_ALPHA = 4;
const int GBUFFER_NPR_PARAM = 5;
const int GBUFFER_NPR_COLOR = 6;

// 渲染属性
const float ALPHA_MODE_OPAQUE = 0.0;
const float ALPHA_MODE_MASK = 1.0;
const float ALPHA_MODE_BLEND = 2.0;

#endif // COMMON_GLSL
