#ifndef MITE_TEXTURE_TYPE
#define MITE_TEXTURE_TYPE

#include "headers/headers.h"

namespace mite {
// 纹理数据类型（像素数据的组件类型）
enum class TextureDataType {
  UNSIGNED_BYTE = GL_UNSIGNED_BYTE,          // 8位无符号字节（最常用）
  FLOAT = GL_FLOAT,                          // 32位浮点（HDR）
  UNSIGNED_SHORT = GL_UNSIGNED_SHORT,        // 16位无符号短整型
  UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,  // 深度模板打包格式
};

// 纹理内部格式（扩展常用格式）
enum class TextureFormat : unsigned int {
  Unknown = 0,

  // 8位无符号归一化格式
  R8 = GL_R8,        // 8位红色通道：遮罩、粗糙度、金属度等
  RG8 = GL_RG8,      // 8位红绿双通道：RG法线贴图
  RGB8 = GL_RGB8,    // 8位RGB：基础颜色（无Alpha）
  RGBA8 = GL_RGBA8,  // 8位RGBA：基础颜色（带Alpha）（最常用）

  // 深度/模板格式
  DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,  // 16位深度
  DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,  // 24位深度
  DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,  // 32位深度
  STENCIL_INDEX1 = GL_STENCIL_INDEX1,        // 1位模板
  STENCIL_INDEX4 = GL_STENCIL_INDEX4,        // 4位模板
  STENCIL_INDEX8 = GL_STENCIL_INDEX8,        // 8位模板
  STENCIL_INDEX16 = GL_STENCIL_INDEX16,      // 16位模板
  DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,    // 24位深度+8位模板

  // sRGB格式（伽马校正）
  SRGB8 = GL_SRGB8,                // sRGB色彩空间
  SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,  // sRGB+Alpha

  // 高精度纹理（GBuffer专用）
  RGB16F = GL_RGB16F,    // HDR RGB (half float)
  RGBA16F = GL_RGBA16F,  // HDR RGBA (half float)
  RGB32F = GL_RGB32F,    // HDR RGB
  RGBA32F = GL_RGBA32F,  // HDR RGBA
};

// 纹理目标类型
enum class TextureTarget {
  TEXTURE_2D = GL_TEXTURE_2D,                                      // 2D纹理（最常用）
  TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,                          // 立方体贴图
  TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,                          // 2D纹理数组
  TEXTURE_3D = GL_TEXTURE_3D,                                      // 3D纹理/体积纹理
  TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,              // 2D多重采样纹理
  TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,              // 立方体贴图数组
  TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,  // 多重采样纹理数组
  TEXTURE_BUFFER = GL_TEXTURE_BUFFER,                              // 缓冲纹理
  TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE  // 矩形纹理（非2的幂次方）
};

// 纹理包装模式（对应OpenGL的wrap参数）
enum class TextureWrapMode {
  Repeat = GL_REPEAT,                   // 默认重复纹理
  ClampToEdge = GL_CLAMP_TO_EDGE,       // 边缘拉伸（防止接缝）
  MirroredRepeat = GL_MIRRORED_REPEAT,  // 镜像重复
  ClampToBorder = GL_CLAMP_TO_BORDER,   // 边框颜色（需要设置边框色）
};

// 纹理过滤模式
enum class TextureFilterMode {
  Nearest = GL_NEAREST,                              // 最近邻采样（像素化风格）
  Linear = GL_LINEAR,                                // 线性过滤（平滑）
  NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,  // 最近邻Mipmap
  LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,    // 线性Mipmap+最近邻层间
  NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,    // 最近邻Mipmap+线性层间
  LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,      // 三线性过滤（最高质量）
};

/**
 * 运行时纹理类型枚举
 * 其中NPR纹理定义：
 * GBuffer_NPRParam：rampThreshold色阶阈值、rampSmoothness色阶平滑度、specularSize高光尺寸、outlineWidth描边宽度
 * GBuffer_NPRColor：shadowTint.rgb阴影色调、rimPower边缘光衰减
 */
enum class RuntimeTextureType {
  None,  // 无效类型

  // G-Buffer 纹理类型
  GBuffer_WorldPosDepth,        // 世界坐标(xyz) + 深度(w，线性深度)(RGBA32F)
  GBuffer_BaseColorMatType,     // 基础颜色(rgb) + 材质类型(a，标志位)(RGBA16F)
  GBuffer_MetallicRoughnessAO,  // 金属度/粗糙度(xy) + AO(z，w保留)(RGBA16F)
  GBuffer_NormalScale,          // 法线(xyz) + 法线缩放(w)(RGBA16F)
  GBuffer_EmissionAlpha,        // 自发光(rgb) + Alpha(a)(RGBA16F)
  GBuffer_NPRParam,             // NPR参数(RGBA16F)
  GBuffer_NPRColor,             // NPR颜色(RGBA16F)

  // 阴影相关
  ShadowMap_Directional,  // 方向光(级联阴影贴图）
  ShadowMap_Point,        // 点光源(立方体阴影贴图)
  ShadowMap_Spot,         // 聚光灯(单面阴影贴图)
  ShadowMap_Area,         // 面光源(平面阴影贴图)

  // 光照着色相关
  Lighting_Diffuse,   // 漫反射着色结果(暂未启用)
  Lighting_Specular,  // 镜面反射着色结果(暂未启用)
  Lighting_Ambient,   // 环境光结果(暂未启用)
  Lighting_Combined,  // 综合光照结果

  // 后期处理相关(暂未启用)
  // PostProcess_Bloom,
  // PostProcess_ToneMapped,
  // PostProcess_Final,

  // 通用类型
  RenderTarget,  // 普通渲染目标
  Depth,         // 深度缓冲
  Stencil,       // 模板缓冲

  // 特殊用途(暂未启用)
  // Debug_View,  // 调试视图纹理
  // UI_Overlay,  // UI覆盖纹理
  // None,        // 不合法纹理
};

/**
 * 外部加载纹理类型枚举
 * 用于从外部文件加载的纹理资源
 */
enum class ExternalTextureType {
  // PBR材质纹理
  BaseColor = 0,      // 基础色纹理
  Normal,             // 法线纹理
  MetallicRoughness,  // 金属粗糙度纹理
  Emissive,           // 自发光纹理
  Occlusion,          // 环境光遮蔽纹理

  // 环境纹理
  EnvironmentMap,  // 环境贴图
  // BRDFLUT,         // BRDF查找表(暂未启用)
  // IrradianceMap,   // 辐照度图(暂未启用)
  // PrefilterMap,    // 预滤波环境图(暂未启用)

  // 后期处理纹理(暂未启用)
  // ColorGradingLUT,  // 色彩分级LUT
  // BloomTexture,     // 泛光纹理
  // SSAOTexture,      // SSAO纹理

  // 自定义纹理(暂未启用)
  // Custom0,  // 自定义纹理0
  // Custom1,  // 自定义纹理1
  // Custom2,  // 自定义纹理2
  // Custom3,  // 自定义纹理3

  Count  // 类型计数
};
};  // namespace mite

#endif