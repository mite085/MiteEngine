#ifndef MITE_ASSET_TYPE
#define MITE_ASSET_TYPE

#include "asset_id.h"
#include "handle_type.h"

namespace mite {
// ------------------------ 纹理相关 ------------------------

// 纹理元数据 - 资源描述和序列化信息
struct TextureMetadata {
  std::string sourcePath;  // 原始文件路径或资源标识

  // 基础属性（用于重新生成TextureSourceData）
  TextureFormat sourceFormat = TextureFormat::RGBA8;  // 数据格式（RGB8/RGBA8等）
  uint32_t originalWidth = 0;                         // 原始图片宽度
  uint32_t originalHeight = 0;                        // 原始图片高度

  // 采样配置（序列化需要）
  TextureWrapMode wrapModeS = TextureWrapMode::Repeat;  // 分离S/T方向包装模式
  TextureWrapMode wrapModeT = TextureWrapMode::Repeat;
  TextureFilterMode minFilter = TextureFilterMode::LinearMipmapLinear;  // 分离缩小/放大过滤
  TextureFilterMode magFilter = TextureFilterMode::LinearMipmapLinear;
  bool generateMipmaps = true;  // 是否生成Mipmap

  // 资源信息
  TextureTarget target = TextureTarget::TEXTURE_2D;  // 纹理目标类型

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

  // 默认构造函数（无效ID）
  TextureAsset() : id(TextureAssetID{}) {}

  // Cache所必须的设定
  using AssetIDType = TextureAssetID;
  TextureAssetID GetID() const
  {
    return id;
  }
  void SetID(const TextureAssetID &newId)
  {
    id = newId;
  }
};


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
  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 模型包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// 模型资产
struct ModelAsset {
  ModelAssetID id;

  // 原始数据存储
  ModelMetadata metadata;

  // 引用的材质ID（而不是MaterialInfo）
  std::vector<MaterialAssetID> materialRefs;

  // GPU相关
  ModelGPUHandle handle;
  std::vector<MeshSectionLODChain> subMeshSection;

  // 默认构造函数（无效ID）
  ModelAsset() : id(ModelAssetID()) {}

  // Cache所必须的设定
  using AssetIDType = ModelAssetID;
  ModelAssetID GetID() const
  {
    return id;
  }
  void SetID(const ModelAssetID &newId)
  {
    id = newId;
  }
};
};  // namespace mite

#endif
