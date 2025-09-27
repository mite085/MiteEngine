#ifndef MITE_HANDLE_TYPE
#define MITE_HANDLE_TYPE

#include "headers/headers.h"
#include "material_param_variant.h"

namespace mite {
// ------------------------ 纹理相关 ------------------------
// 纹理数据类型（像素数据的组件类型）
enum class TextureDataType {
  UNSIGNED_BYTE = GL_UNSIGNED_BYTE,          // 8位无符号字节（最常用）
  FLOAT = GL_FLOAT,                          // 32位浮点（HDR）
  UNSIGNED_SHORT = GL_UNSIGNED_SHORT,        // 16位无符号短整型
  UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,  // 深度模板打包格式
};
// 纹理内部格式（扩展常用格式）
enum class TextureFormat {
  Unknown = 0,

  // 8位无符号归一化格式
  R8 = GL_R8,        // 8位红色通道：遮罩、粗糙度、金属度等
  RG8 = GL_RG8,      // 8位红绿双通道：RG法线贴图
  RGB8 = GL_RGB8,    // 8位RGB：基础颜色（无Alpha）
  RGBA8 = GL_RGBA8,  // 8位RGBA：基础颜色（带Alpha）（最常用）

  // 深度/模板格式
  DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,  // 16位深度
  DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,    // 24位深度+8位模板

  // sRGB格式（伽马校正）
  SRGB8 = GL_SRGB8,                // sRGB色彩空间
  SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,  // sRGB+Alpha

  // HDR支持，留待后续扩展
  // RGB16F,   // HDR RGB (half float)
  // RGBA16F,  // HDR RGBA (half float)
  // RGB32F,   // HDR RGB
  // RGBA32F,  // HDR RGBA
};
// 纹理目标类型
enum class TextureTarget {
  TEXTURE_2D = GL_TEXTURE_2D,              // 2D纹理（最常用）
  TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,  // 立方体贴图
  TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,  // 2D纹理数组
  TEXTURE_3D = GL_TEXTURE_3D,              // 3D纹理/体积纹理
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
// 纹理GPU句柄
struct TextureGPUHandle {
  uintptr_t apiHandle = 0;  // 底层驱动句柄（OpenGL的GLuint）
};

// 纹理数据来源（Renderer模块专用的过渡型数据格式）
struct TextureSourceData {
  // 核心数据
  std::vector<uint8_t> pixelData;               // 原始像素数据（只读指针）
  uint32_t width = 0;                           // 纹理宽度
  uint32_t height = 0;                          // 纹理高度
  TextureFormat format = TextureFormat::RGBA8;  // 数据格式（RGB8/RGBA8等）

  // 纹理目标
  TextureTarget target = TextureTarget::TEXTURE_2D;  // 纹理目标类型

  // 采样参数
  TextureWrapMode wrapModeS = TextureWrapMode::Repeat;  // 分离S/T方向包装模式
  TextureWrapMode wrapModeT = TextureWrapMode::Repeat;
  TextureFilterMode minFilter = TextureFilterMode::LinearMipmapLinear;  // 分离缩小/放大过滤
  TextureFilterMode magFilter = TextureFilterMode::Linear;

  // Mipmap设置
  bool generateMipmaps = true;     // 是否生成Mipmap
  uint32_t existingMipLevels = 1;  // 源数据已有的mip层级数
};

// 纹理实例 - 纯粹的运行时渲染对象
struct TextureInstance {
  TextureGPUHandle gpuHandle;  // GPU资源句柄
  TextureTarget target;        // 纹理目标类型
  TextureFormat format;        // 内部格式
  uint32_t width;              // 实际纹理宽度
  uint32_t height;             // 实际纹理高度
  uint32_t mipLevels;          // Mipmap层级数

  // 采样状态
  TextureWrapMode wrapModeS;
  TextureWrapMode wrapModeT;
  TextureFilterMode minFilter;
  TextureFilterMode magFilter;
};

// ------------------------ 材质相关 ------------------------
// 透明度模式（GLTF标准）
enum class AlphaMode {
  OPAQUE = 0,  // 不透明材质
  MASK = 1,  // 透明度裁剪（硬边缘，仅支持0和1的透明度，无需考虑渲染顺序）
  BLEND = 2  // 透明度混合（软边缘，需要从后向前渲染）
};

// 运行时纹理槽位纹理槽，包含纹理GPUHandle和缩放偏移，仅渲染前的材质Apply时需要
struct TextureGPUSlot {
  TextureGPUHandle gpuHandle;
  glm::vec2 scale = glm::vec2(1.0f);   // 纹理缩放
  glm::vec2 offset = glm::vec2(0.0f);  // 纹理偏移

  TextureGPUSlot() = default;
  TextureGPUSlot(TextureGPUHandle handle, const glm::vec2 &s, const glm::vec2 &o)
      : gpuHandle(handle), scale(s), offset(o)
  {
  }
};

// 材质数据来源（MaterialSystem专用的过渡型数据格式）
struct MaterialSourceData {
  // 核心标识信息
  std::string name;               // 材质名称
  std::string templateName;       // 对应的MaterialTemplate名称（用于索引模板）

  // 通用参数存储
  std::unordered_map<std::string, UniformVariant> parameters;

  // 运行时纹理槽位（包含GPU句柄，支持Instance和offset）
  std::unordered_map<std::string, TextureGPUSlot> textureSlots;

  // 渲染属性
  AlphaMode alphaMode = AlphaMode::OPAQUE;  // 透明度模式
  float alphaCutoff = 0.5f;                 // Alpha测试阈值
  bool doubleSided = false;                 // 是否双面渲染
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
  uint32_t materialIndex;  // 材质索引
  uint32_t lodLevel;       // LOD级别，0表示原始LOD
};

// 子网格 LOD 链结构
struct MeshSectionLODChain {
  MeshSection baseSection;               // 基础 LOD (level 0)
  std::vector<MeshSection> lodSections;  // 其他 LOD 级别 (level 1+)
};

// ------------------------ 模型相关 ------------------------

// 模型数据来源（Renderer模块专用的过渡型数据格式）
struct ModelSourceData {
  // 构建GPUHandle的核心部分
  std::vector<uint8_t> mergedVertexData;  // 合并后的顶点数据
  std::vector<uint32_t> mergedIndices;    // 合并后的索引数据
  VertexLayout layout;                    // 顶点布局(所有子网格共享)

  // 需要传递给GPUHandle的信息
  std::string path;        // 文件原始路径（用于调试）
  glm::vec3 modelBboxMin;  // 模型级包围盒
  glm::vec3 modelBboxMax;
};

// 模型GPU句柄
struct ModelGPUHandle {
  // GPUHandle的核心部分
  uintptr_t vertexArray = 0;   // 整个Model的VAO
  uintptr_t vertexBuffer = 0;  // 整个Model的VBO
  uintptr_t indexBuffer = 0;   // 整个Model的EBO

  // 从上层收到的信息
  std::string path;   // 文件原始路径（用于调试打印）
  glm::vec3 bboxMin;  // 模型级包围盒
  glm::vec3 bboxMax;
};

// ------------------------ 着色器相关 ------------------------

// 着色器GPU句柄
struct ShaderGPUHandle {
  uintptr_t programId = 0;  // OpenGL程序对象ID

  uintptr_t vertexShader = 0;    // 顶点着色器
  uintptr_t fragmentShader = 0;  // 片段着色器
  uintptr_t geometryShader = 0;  // 几何着色器
  uintptr_t computeShader = 0;   // 计算着色器
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
