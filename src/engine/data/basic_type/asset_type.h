#ifndef MITE_ASSET_TYPE
#define MITE_ASSET_TYPE

#include "asset_id.h"
#include "handle_type.h"
#include "uuid/mite_uuid.h"
#include <glm/glm.hpp>

namespace mite {
// ------------------------ 纹理相关 ------------------------
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

// 纹理元数据 - 资源描述和序列化信息
struct TextureMetadata {
  std::string sourcePath;  // 原始文件路径或资源标识

  // 基础属性（用于重新生成TextureSourceData）
  TextureFormat sourceFormat;  // 源数据格式
  uint32_t originalWidth;      // 原始图片宽度
  uint32_t originalHeight;     // 原始图片高度
  uint32_t channelCount;       // 原始通道数

  // 采样配置（序列化需要）
  TextureWrapMode wrapModeS;
  TextureWrapMode wrapModeT;
  TextureFilterMode minFilter;
  TextureFilterMode magFilter;
  bool generateMipmaps;

  // 资源信息
  TextureTarget target;     // 纹理目标类型
  bool isSRGB;              // 是否sRGB色彩空间
  std::string compression;  // 压缩格式标识（如有）

  TextureSourceData generateSourceData(std::vector<uint8_t> &&pixelData) const
  {
    TextureSourceData sourceData;

    // 设置核心数据
    sourceData.pixelData = std::move(pixelData);  // 移动语义，避免拷贝
    sourceData.width = originalWidth;
    sourceData.height = originalHeight;
    sourceData.format = sourceFormat;
    sourceData.target = target;

    // 设置采样参数
    sourceData.wrapModeS = wrapModeS;
    sourceData.wrapModeT = wrapModeT;
    sourceData.minFilter = minFilter;
    sourceData.magFilter = magFilter;
    sourceData.generateMipmaps = generateMipmaps;

    // 设置Mipmap（默认从原始数据推断）
    sourceData.existingMipLevels = 1;  // stbi_load总是返回单级Mipmap

    return sourceData;
  }
};

// 纹理资产
struct TextureAsset {
  TextureAssetID id;         // ID
  TextureMetadata metadata;  // 元数据
  TextureInstance instance;  // 纹理实例

  TextureAsset() : id(TextureAssetID{}) {}
};

// ------------------------ 材质相关 ------------------------


// ------------------------ 模型/网格相关 ------------------------

// 子网格数据（API无关的几何数据）
struct MeshData {
  std::vector<uint8_t> vertexData;  // 原始顶点字节流
  std::vector<uint32_t> indices;    // 索引数据
  VertexLayout layout;              // 顶点结构描述
  uint32_t materialIndex = 0;       // 关联的材质索引

  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 子网格局部包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);

  uint32_t lodLevel = 0;  // 0表示原始LOD
};

// 子网格 LOD 链结构
struct MeshDataLODChain {
  MeshData baseSection;               // 基础 LOD (level 0)
  std::vector<MeshData> lodSections;  // 其他 LOD 级别 (level 1+)
};

// 模型元数据（临时结构，用于加载）
struct ModelMetadata {
  std::string path;
  std::vector<MaterialInfo> materials;            // 材质信息
  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 模型包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// 模型资产
struct ModelAsset {
  ModelAssetID id;

  // 原始数据存储
  ModelMetadata metadata;
  std::vector<MeshDataLODChain> subMeshData;

  // 引用的材质ID（而不是MaterialInfo）
  std::vector<MaterialAssetID> materialIDs;

  // GPU相关
  ModelGPUHandle handle;
  std::vector<MeshSectionLODChain> subMeshSection;

  ModelAsset() : id(ModelAssetID()) {}
};
};  // namespace mite

#endif
