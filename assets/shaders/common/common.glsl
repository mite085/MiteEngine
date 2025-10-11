// 通用常量和结构体定义
#ifndef COMMON_GLSL
#define COMMON_GLSL

// 材质类型枚举
const uint MATERIAL_TYPE_PBR = 0u;
const uint MATERIAL_TYPE_NPR = 1u;

// Alpha模式枚举
const uint ALPHA_MODE_OPAQUE = 0u;
const uint ALPHA_MODE_MASK = 1u;
const uint ALPHA_MODE_BLEND = 2u;

// 投影类型
const uint PROJECTION_PERSPECTIVE = 1u;
const uint PROJECTION_ORTHOGRAPHIC = 0u;

// 光源类型枚举
const uint LIGHT_TYPE_POINT = 0u;
const uint LIGHT_TYPE_SPOT = 1u;
const uint LIGHT_TYPE_DIRECTIONAL = 2u;
const uint LIGHT_TYPE_AREA_RECT = 3u;
const uint LIGHT_TYPE_AREA_ELLIPSE = 4u;
// 面光源形状枚举（矩形和椭圆）
const uint AREA_LIGHT_SHAPE_RECTANGLE = 0u;
const uint AREA_LIGHT_SHAPE_ELLIPSE = 1u;

// GBuffer Output索引定义 - 与C++端GBuffer::GBufferIndex对应
const int GBUFFER_WORLDPOS_DEPTH = 0;
const int GBUFFER_BASECOLOR_MATTYPE = 1;
const int GBUFFER_METALLICROUGHNESS_AO = 2;
const int GBUFFER_NORMAL_SCALE = 3;
const int GBUFFEE_EMISSION_ALPHA = 4;
const int GBUFFER_NPR_PARAM = 5;
const int GBUFFER_NPR_COLOR = 6;

#endif
