#ifndef MITE_HANDLE_TYPE
#define MITE_HANDLE_TYPE

#include <glm/glm.hpp>
#include <variant>
#include <vector>
#include <glad.h>
#include <string>

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
  std::string path;         // 文件原始路径	
  uintptr_t apiHandle = 0;  // 底层驱动句柄（OpenGL的GLuint或Vulkan的VkImage）
};

// 纹理数据来源（Renderer模块专用的过渡型数据格式）
struct TextureSourceData {
  std::string path;              // 文件原始路径
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
  uint32_t vertexOffset;  // Mesh在Model的VAO中的偏移量
  uint32_t indexOffset;   // Mesh在Model的VBO中的偏移量
  uint32_t vertexCount;   // Mesh的顶点数量
  uint32_t indexCount;    // Mesh的索引数量
  glm::vec3 bboxMin;      // 网格级包围盒
  glm::vec3 bboxMax;

  uint32_t lodLevel;  // 添加LOD级别
};

// ------------------------ 模型相关 ------------------------

// 模型数据来源（Renderer模块专用的过渡型数据格式）
struct ModelSourceData {
  std::string path;                       // 文件原始路径
  std::vector<uint8_t> mergedVertexData;  // 合并后的顶点数据
  std::vector<uint32_t> mergedIndices;    // 合并后的索引数据
  std::vector<MeshSection> sections;      // 子网格分段信息
  VertexLayout layout;                    // 顶点布局(所有子网格共享)
  glm::vec3 modelBboxMin;                 // 模型级包围盒
  glm::vec3 modelBboxMax;
};

// 模型GPU句柄
struct ModelGPUHandle {
  std::string path;                    // 文件原始路径
  uintptr_t vertexArray;               // 整个Model的VAO
  uintptr_t vertexBuffer;              // 整个Model的VBO
  uintptr_t indexBuffer;               // 整个Model的EBO
  std::vector<MeshSection> subMeshes;  // 子Mesh信息
  glm::vec3 bboxMin;                   // 模型级包围盒
  glm::vec3 bboxMax;
};

// ------------------------ 帧缓冲相关 ------------------------

// 帧缓冲附件类型枚举
enum class FrameBufferAttachmentType {
  Color = 0,    // 颜色附件
  Depth,        // 深度附件
  Stencil,      // 模板附件
  DepthStencil  // 深度模板组合附件
};

// 帧缓冲附件规格结构体
struct FrameBufferAttachmentSpec {
  FrameBufferAttachmentType type = FrameBufferAttachmentType::Color;
  GLenum internalFormat = GL_RGBA8;    // 内部格式
  GLenum format = GL_RGBA;             // 数据格式
  GLenum dataType = GL_UNSIGNED_BYTE;  // 数据类型
  bool generateMipmaps = false;        // 是否生成mipmaps
};

// 帧缓冲规格结构体
struct FrameBufferSpec {
  uint32_t width = 1280;                               // 默认宽度
  uint32_t height = 720;                               // 默认高度
  std::vector<FrameBufferAttachmentSpec> attachments;  // 附件列表
  uint32_t samples = 1;  // 多重采样数(默认为1，即不启用)
};
};  // namespace mite

#endif
