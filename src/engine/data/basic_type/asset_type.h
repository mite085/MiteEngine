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
  TextureTarget target;  // 纹理目标类型

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

// 材质纹理槽位定义（GLTF PBR标准）
struct MaterialTextureSlot {
  TextureAssetID textureAssetId;       // 纹理资产ID
  glm::vec2 scale = glm::vec2(1.0f);   // 纹理缩放
  glm::vec2 offset = glm::vec2(0.0f);  // 纹理偏移

  MaterialTextureSlot() = default;
  explicit MaterialTextureSlot(TextureAssetID id) : textureAssetId(id) {}
};

// 材质元数据 - 基于UniformVariant的通用参数存储（目前支持GLTF，后续扩展MaterialX）
struct MaterialMetadata {
  std::string sourcePath;         // 源文件路径

  // 基础信息
  std::string name;          // 材质名称
  std::string templateName;  // 对应的MaterialTemplate名称（用于索引材质）

  // 通用参数存储（支持GLTF PBR和未来MaterialX）
  std::unordered_map<std::string, UniformVariant> parameters;

  // 通用纹理槽位存储
  std::unordered_map<std::string, MaterialTextureSlot> textureSlots;

  // 渲染属性
  AlphaMode alphaMode = AlphaMode::OPAQUE;  // 透明度模式
  float alphaCutoff = 0.5f;                 // Alpha测试阈值
  bool doubleSided = false;                 // 是否双面渲染

  // 来源特定信息（仅存储原始信息，不包含任何操作）
  struct GLTFSourceInfo {
    std::string gltfFilePath;  // GLTF文件路径
    uint32_t materialIndex;    // 材质在文件中的索引
  };
  // struct MaterialXSourceInfo {
  //   std::string materialxFilePath;  // MaterialX文件路径
  //   std::string nodeDefName;        // MaterialX节点定义名称
  // };
  //  使用variant支持不同来源的扩展信息
  std::variant<GLTFSourceInfo, /*MaterialXSourceInfo*/> sourceInfo;

  MaterialMetadata() {}
  /**
   * @brief 生成材质源数据，过滤掉与渲染无关的信息
   * @param textureResolver 纹理解析回调函数，用于将TextureAssetID转换为TextureGPUHandle
   * @return 包含所有用于创建材质实例信息的MaterialSourceData
   */
  MaterialSourceData generateSourceData(
      std::function<TextureInstance(const TextureAssetID &)> textureResolver) const
  {
    MaterialSourceData sourceData;

    // 复制核心标识信息
    sourceData.templateName = templateName;
    sourceData.name = name;

    // 复制参数
    sourceData.parameters = parameters;

    // 转换纹理槽位：TextureAssetID -> TextureGPUHandle
    for (const auto &[slotName, textureSlot] : textureSlots) {
      if (textureSlot.textureAssetId.IsValid()) {
        // 使用回调函数解析纹理资产ID为纹理实例
        TextureInstance textureInstance = textureResolver(textureSlot.textureAssetId);

        // 创建运行时纹理槽位
        sourceData.textureSlots[slotName] = TextureGPUSlot(
            textureInstance.gpuHandle, textureSlot.scale, textureSlot.offset);
      }
    }

    // 复制渲染属性
    sourceData.alphaMode = alphaMode;
    sourceData.alphaCutoff = alphaCutoff;
    sourceData.doubleSided = doubleSided;

    return sourceData;
  }
};

// 材质资产
struct MaterialAsset {
  MaterialAssetID id;         // 资产ID（由Asset管理）
  MaterialMetadata metadata;  // 材质元数据

  // 注意：不直接存储MaterialInstance，由MaterialSystem管理

  MaterialAsset() : id(MaterialAssetID{}) {}
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
