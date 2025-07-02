#ifndef MITE_ASSET_TYPE
#define MITE_ASSET_TYPE

#include "headers/headers.h"

namespace mite {
// --- 基础类型别名 ---
using AssetID = uuids::uuid;  // 资源唯一标识符（用UUID生成）

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

// 纹理元数据（描述纹理的属性和来源）
struct TextureMetadata {
  std::string path;                             // 资源路径（用于热重载识别）
  TextureFormat format = TextureFormat::RGBA8;  // 数据格式
  int width = 0;                                // 宽度（像素）
  int height = 0;                               // 高度
  int channels = 4;                             // 颜色通道数（RGB=3, RGBA=4）
  bool isHDR = false;                           // 是否是HDR纹理
};

// 纹理数据
// 存储内容为：uint8_t[]纹理数组，void (*)(uint8_t *)析构方法
struct TetxureData {
  std::unique_ptr<uint8_t[], void (*)(uint8_t *)> textureData;
};
// ------------------------ 模型/网格相关 ------------------------
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

// 子网格数据（API无关的几何数据）
struct MeshData {
  std::vector<uint8_t> vertexData;  // 原始顶点字节流
  std::vector<uint32_t> indices;    // 索引数据
  VertexLayout layout;              // 顶点结构描述
  uint32_t materialIndex = 0;       // 关联的材质索引

  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 子网格局部包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// 模型元数据
struct ModelMetadata {
  std::string path;
  std::vector<std::string> materialPaths;         // 材质路径引用
  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 模型包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// ------------------------ 资源句柄 --------------------
// 纹理逻辑句柄
struct TextureAsset {
  AssetID id;  // 唯一标识符
  TextureMetadata metadata;
  TetxureData textureData;
};
// 纹理GPU句柄
struct TextureGPUHandle {
  uintptr_t apiHandle = 0;  // 底层驱动句柄（OpenGL的GLuint或Vulkan的VkImage）
};
// 模型逻辑句柄
struct ModelAsset {
  AssetID id;
  ModelMetadata metadata;
  std::vector<MeshData> subMeshes;  // 包含GPU资源的子网格集合
};
// 模型GPU句柄
struct ModelGPUHandle {
  std::vector<MeshGPUHandle> subMeshes;  // 每个子网格的GPU资源
};
};  // namespace mite

#endif
