#ifndef MITE_MATERIAL_TYPE
#define MITE_MATERIAL_TYPE

#include "asset_id.h"
#include "basic_instance/material_instance.h"
#include "handle_type.h"
#include "material_param_variant.h"

namespace mite {
// ------------------------ 材质相关 ------------------------

// 材质纹理槽位定义（GLTF PBR标准）
struct MaterialTextureSlot {
  TextureAssetID textureAssetId;                            // 纹理资产ID
  TextureTarget textureTarget = TextureTarget::TEXTURE_2D;  // 纹理类型（默认2D纹理）
  glm::vec2 scale = glm::vec2(1.0f);                        // 纹理缩放
  glm::vec2 offset = glm::vec2(0.0f);                       // 纹理偏移

  MaterialTextureSlot() = default;
  explicit MaterialTextureSlot(TextureAssetID id, TextureTarget target) : textureAssetId(id), textureTarget(target) {}
};

// 材质数据来源（MaterialSystem专用的过渡型数据格式）
struct MaterialSourceData {
  // 核心标识信息
  std::string name;   // 材质名称
  MaterialType type;  // 对应Type

  // 通用参数存储
  std::unordered_map<std::string, UniformVariant> parameters;

  // 运行时纹理槽位（包含GPU句柄，支持Instance和offset）
  std::unordered_map<std::string, TextureGPUSlot> textureSlots;

  // 渲染属性
  AlphaMode alphaMode = AlphaMode::OPAQUE;  // 透明度模式
  float alphaCutoff = 0.5f;                 // Alpha测试阈值
  bool doubleSided = false;                 // 是否双面渲染
};

// 材质元数据 - 基于UniformVariant的通用参数存储（目前支持GLTF，后续扩展MaterialX）
struct MaterialMetadata {
  std::string sourcePath;  // 源文件路径

  // 基础信息
  std::string name;   // 材质名称
  MaterialType type;  // 对应Type

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
    std::string gltfFilePath;    // GLTF文件路径
    uint32_t materialIndex = 0;  // 材质在文件中的索引
  };
  // struct MaterialXSourceInfo {
  //   std::string materialxFilePath;  // MaterialX文件路径
  //   std::string nodeDefName;        // MaterialX节点定义名称
  // };
  //  使用variant支持不同来源的扩展信息
  std::variant<GLTFSourceInfo /*, MaterialXSourceInfo*/> sourceInfo;

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
    sourceData.type = type;
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
            textureInstance.gpuHandle, textureInstance.target, textureSlot.scale, textureSlot.offset);
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
  MaterialAssetID id;                          // 资产ID（由Asset管理）
  MaterialMetadata metadata;                   // 材质元数据（包含纹理ID引用）
  std::shared_ptr<MaterialInstance> instance;  // 材质实例（包含纹理实例SharedPtr）

  // 默认构造函数（无效ID）
  MaterialAsset() : id(MaterialAssetID{}), metadata(), instance(nullptr) {}

  // Cache所必须的设定
  using AssetIDType = MaterialAssetID;
  MaterialAssetID GetID() const { return id; }
  void SetID(const MaterialAssetID &newId) { id = newId; }
};
};  // namespace mite

#endif
