#ifndef MITE_HANDLE_TYPE
#define MITE_HANDLE_TYPE

#include <variant>
#include <vector>
#include <glm/glm.hpp>

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

// 子网格GPU信息（仅仅包含Offset数据）
struct MeshSection {
  uint32_t vertexOffset;
  uint32_t indexOffset;
  uint32_t vertexCount;
  uint32_t indexCount;
  glm::vec3 bboxMin;
  glm::vec3 bboxMax;
};

// 模型GPU句柄
struct ModelGPUHandle {
  uintptr_t vertexArray;               // 整个Model的VAO
  uintptr_t vertexBuffer;              // 整个Model的VBO
  uintptr_t indexBuffer;               // 整个Model的EBO
  std::vector<MeshSection> subMeshes;  // 子Mesh信息
};

// ------------------------ 模型相关 ------------------------


// 模型数据来源（Renderer模块专用）
struct ModelSourceData {
  std::vector<uint8_t> mergedVertexData;  // 合并后的顶点数据
  std::vector<uint32_t> mergedIndices;    // 合并后的索引数据
  std::vector<MeshSection> sections;      // 子网格分段信息
  VertexLayout layout;                    // 顶点布局(所有子网格共享)
  glm::vec3 modelBboxMin;                 // 模型级包围盒
  glm::vec3 modelBboxMax;
};

};  // namespace mite

#endif
