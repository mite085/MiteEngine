#ifndef MITE_RENDER_HANDLE_TYPES
#define MITE_RENDER_HANDLE_TYPES

#include "headers/headers.h"

namespace mite {
// ------------------------ 纹理相关 ------------------------

// 纹理格式枚举
enum class TextureFormat {
  Unknown,
  RGB8,     // 8-bit per channel RGB
  RGBA8,    // 8-bit RGBA with alpha
  RGB16F,   // HDR RGB (half float)
  RGBA16F,  // HDR RGBA
};
// 纹理包装模式（对应OpenGL/Vulkan的wrap参数）
enum class TextureWrapMode {
  Repeat,         // 默认重复纹理
  ClampToEdge,    // 边缘拉伸
  MirroredRepeat  // 镜像重复
};

// 纹理过滤模式
enum class TextureFilterMode {
  Nearest,     // 最近邻采样（像素化风格）
  Linear,      // 线性过滤（平滑）
  Anisotropic  // 各向异性过滤（需硬件支持）
};

// 纹理GPU句柄
struct TextureGPUHandle {
  uintptr_t apiHandle = 0;  // 底层驱动句柄（OpenGL的GLuint或Vulkan的VkImage）
};

// 纹理数据来源（Renderer模块专用）
struct TextureSourceData {
  const uint8_t *pixelData;      // 原始像素数据（只读指针）
  int width;                     // 纹理宽度
  int height;                    // 纹理高度
  TextureFormat format;          // 数据格式（RGB8/RGBA8等）
  TextureWrapMode wrapMode;      // 包装模式
  TextureFilterMode filterMode;  // 过滤模式
  bool generateMipmaps;          // 是否生成Mipmap
};

// ------------------------ 网格相关 ------------------------

// 顶点属性标志（描述顶点结构）
enum class VertexAttribute {
  Position,
  Normal,
  TexCoord,
  Tangent,
  Bitangent,
};

// 顶点格式描述（替代硬编码的Vertex结构体）
struct VertexLayout {
  std::vector<VertexAttribute> attributes;
  uint32_t stride = 0;  // 顶点总字节数
};

// 子网格GPU模型句柄
struct MeshGPUHandle {
  uintptr_t vertexArray = 0;   // 顶点数组索引
  uintptr_t vertexBuffer = 0;  // 顶点缓冲区
  uintptr_t indexBuffer = 0;   // 索引缓冲区
  uint32_t vertexCount = 0;    // 顶点数量
  uint32_t indexCount = 0;     // 索引数量
};

// 子网格数据来源（Renderer模块专用）
struct MeshSourceData {
  const uint8_t *vertexData;  // 顶点数据指针（只读）
  const uint32_t *indices;    // 索引数据指针（只读）
  uint32_t vertexCount;       // 顶点数量
  uint32_t indexCount;        // 索引数量
  VertexLayout layout;        // 顶点布局
  glm::vec3 bboxMin;          // 子网格包围盒
  glm::vec3 bboxMax;
};

// ------------------------ 模型相关 ------------------------

// 模型GPU句柄
struct ModelGPUHandle {
  std::vector<MeshGPUHandle> subMeshes;  // 每个子网格的GPU资源
};

// 模型数据来源（Renderer模块专用）
struct ModelSourceData {
  std::vector<MeshSourceData> subMeshes;  // 子网格集合
  glm::vec3 modelBboxMin;                 // 模型级包围盒
  glm::vec3 modelBboxMax;
};


};  // namespace mite

#endif
