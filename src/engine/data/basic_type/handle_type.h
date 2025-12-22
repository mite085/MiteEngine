#ifndef MITE_HANDLE_TYPE
#define MITE_HANDLE_TYPE

#include "texture_type.h"

namespace mite {
// ------------------------ 材质相关 ------------------------
// 材质类型枚举
enum class MaterialType : uint32_t {
  PBR = 1,       // 基于物理的渲染材质
  EMISSION = 2,  // 自发光材质（功能性测试专用）

  CUSTOM = 255  // 自定义材质
};

// ------------------------ 纹理相关 ------------------------
// 纹理创建信息（GBuffer、ShadowMap等运行时纹理专用）
struct TextureCreateInfo {
  uint32_t width = 0;                                // 纹理宽度
  uint32_t height = 0;                               // 纹理高度
  TextureFormat format = TextureFormat::RGBA8;       // 数据格式（RGB8/RGBA8等）
  TextureTarget target = TextureTarget::TEXTURE_2D;  // 纹理目标类型
  TextureWrapMode wrapModeS = TextureWrapMode::Repeat;  // 分离S/T方向包装模式
  TextureWrapMode wrapModeT = TextureWrapMode::Repeat;
  TextureFilterMode minFilter = TextureFilterMode::Linear;  // 分离缩小/放大过滤
  TextureFilterMode magFilter = TextureFilterMode::Linear;
  bool generateMipmaps = false;  // 是否生成Mipmap（运行时纹理默认不生成mipmap）
  uint32_t arrayLayers = 1;      // 纹理层数（如果是数组纹理）
};

// 纹理数据来源（TextureAsset外部载入纹理专用）
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
  TextureFilterMode minFilter =
      TextureFilterMode::LinearMipmapLinear;  // 分离缩小/放大过滤
  TextureFilterMode magFilter = TextureFilterMode::Linear;

  // Mipmap设置
  bool generateMipmaps = true;     // 是否生成Mipmap（默认生成）
  uint32_t existingMipLevels = 1;  // 源数据已有的mip层级数
};

// 纹理GPU句柄
struct TextureGPUHandle {
  uintptr_t apiHandle = 0;  // 底层驱动句柄（OpenGL的GLuint）
};

// 纹理实例（TextureAsset外部载入纹理专用）
struct TextureInstance {
  TextureGPUHandle gpuHandle;                        // GPU资源句柄
  TextureTarget target = TextureTarget::TEXTURE_2D;  // 纹理目标类型
  TextureFormat format = TextureFormat::RGBA8;       // 内部格式
  uint32_t width = 0;                                // 实际纹理宽度
  uint32_t height = 0;                               // 实际纹理高度

  // 采样状态
  TextureWrapMode wrapModeS = TextureWrapMode::Repeat;
  TextureWrapMode wrapModeT = TextureWrapMode::Repeat;
  TextureFilterMode minFilter = TextureFilterMode::LinearMipmapLinear;
  TextureFilterMode magFilter = TextureFilterMode::LinearMipmapLinear;
};

// ------------------------ 材质相关 ------------------------
// 透明度模式（GLTF标准）
enum class AlphaMode {
  OPAQUE = 0,  // 不透明材质
  MASK = 1,    // 透明度裁剪（硬边缘，仅支持0和1的透明度，无需考虑渲染顺序）
  BLEND = 2    // 透明度混合（软边缘，需要从后向前渲染）
};

// 运行时纹理槽位纹理槽，包含纹理GPUHandle和缩放偏移，仅渲染前的材质Apply时需要
struct TextureGPUSlot {
  TextureGPUHandle gpuHandle;
  TextureTarget target =
      TextureTarget::TEXTURE_2D;       // 纹理目标类型（默认2D纹理）
  glm::vec2 scale = glm::vec2(1.0f);   // 纹理缩放
  glm::vec2 offset = glm::vec2(0.0f);  // 纹理偏移

  TextureGPUSlot() = default;
  TextureGPUSlot(TextureGPUHandle handle, TextureTarget target,
                 const glm::vec2 &s, const glm::vec2 &o)
      : gpuHandle(handle), target(target), scale(s), offset(o) {}
};

// ------------------------ 网格相关 ------------------------

// 顶点属性标志（描述顶点结构，注意顺序）
enum class VertexAttribute : uint32_t {
  Position = 0,   // 固定 location 0
  Normal = 1,     // 固定 location 1
  TexCoord = 2,   // 固定 location 2
  Tangent = 3,    // 固定 location 3
  Bitangent = 4,  // 固定 location 4
  Count           // 计数位
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

  uint32_t lodLevel;  // LOD级别，0表示原始LOD
};

// 子网格 LOD 链结构
struct MeshSectionLODChain {
  std::string name;                       // 网格体名称
  glm::mat4 transform = glm::mat4(1.0f);  // 变换矩阵
  uint32_t materialIndex;                 // 材质索引
  MeshSection baseSection;                // 基础 LOD (level 0)
  std::vector<MeshSection> lodSections;   // 其他 LOD 级别 (level 1+)
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
  std::string path;                        // 文件原始路径（用于调试打印）
  glm::vec3 bboxMin = glm::vec3(FLT_MAX);  // 模型级包围盒
  glm::vec3 bboxMax = glm::vec3(-FLT_MAX);
};

// ------------------------ 着色器相关 ------------------------

// 着色器GPU句柄
// 注意：Shaderc 编译为 SPIR-V，不需要保留单独的着色器对象句柄
struct ShaderGPUHandle {
  uintptr_t programId = 0;  // OpenGL程序对象ID
};
};  // namespace mite

#endif
