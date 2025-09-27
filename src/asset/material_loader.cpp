#include "material_loader.h"
#include "texture_loader.h"
#include "material_templates/material_template_pure_color.h"
#include "material_templates/material_template_gltf_pbr.h"
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/scene.h>

namespace mite {

std::vector<std::shared_ptr<MaterialAsset>> MaterialLoader::LoadMaterialsFromGLTF(
    const aiScene *scene,
    const std::string &modelPath,
    const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures)
{

  std::vector<std::shared_ptr<MaterialAsset>> materials;

  if (!scene) {
    LOG_ERROR("Null scene provided for material loading");
    return materials;
  }

  // 创建纹理解析器回调函数
  auto textureResolver = [&loadedTextures](const TextureAssetID &id) -> TextureInstance {
    for (const auto &texture : loadedTextures) {
      if (texture && texture->id == id) {
        return texture->instance;
      }
    }
    return TextureInstance{};  // 返回空的纹理实例
  };

  // 处理场景中的所有材质
  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    aiMaterial *aiMat = scene->mMaterials[i];

    auto material = ProcessGLTFMaterial(aiMat, i, modelPath, loadedTextures);
    if (material) {
      materials.push_back(material);
      LOG_INFO("Successfully loaded material: " + material->metadata.name);

      // 生成MaterialSourceData并发布事件
      MaterialSourceData sourceData = material->metadata.generateSourceData(textureResolver);
      // 发布材质加载事件
      EventBus::Publish(MaterialLoadedEvent(sourceData));
    }
    else {
      LOG_WARN("Failed to process material at index: " + std::to_string(i));
      // 创建默认材质作为回退
      auto fallbackMaterial = CreatePureColorMaterial("Fallback_Material_" + std::to_string(i));
      materials.push_back(fallbackMaterial);

      // 为默认材质也发布事件
      MaterialSourceData sourceData = fallbackMaterial->metadata.generateSourceData(
          textureResolver);
      EventBus::Publish(MaterialLoadedEvent(sourceData));
    }
  }

  // 如果没有材质，创建一个PureColor材质
  if (materials.empty()) {
    LOG_INFO("No materials found in scene, creating default material");
    materials.push_back(CreatePureColorMaterial("Default_Material"));

    // 发布默认材质事件
    MaterialSourceData sourceData = materials.back()->metadata.generateSourceData(textureResolver);
    EventBus::Publish(MaterialLoadedEvent(sourceData));
  }

  return materials;
}

std::shared_ptr<MaterialAsset> MaterialLoader::ProcessGLTFMaterial(
    aiMaterial *aiMat,
    uint32_t materialIndex,
    const std::string &modelPath,
    const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures)
{

  if (!aiMat) {
    LOG_ERROR("Null aiMaterial provided");
    return nullptr;
  }

  auto materialAsset = std::make_shared<MaterialAsset>();

  // 生成材质ID和名称
  std::string materialName = GenerateMaterialName(aiMat, materialIndex, modelPath);
  materialAsset->id = MaterialAssetID{UUIDGenerator::Generate(materialName.c_str())};

  // 设置基础元数据
  materialAsset->metadata.sourcePath = modelPath;
  materialAsset->metadata.name = materialName;
  materialAsset->metadata.templateName = GLTFPBRMaterialTemplate::StaticType();  // 对应的材质模板

  // 提取GLTF PBR参数
  ExtractPBRParameters(aiMat, materialAsset->metadata);

  // 提取纹理引用
  ExtractTextureReferences(aiMat, materialAsset->metadata, modelPath, loadedTextures);

  // 设置GLTF源信息
  materialAsset->metadata.sourceInfo = MaterialMetadata::GLTFSourceInfo{
      modelPath,     // gltfFilePath
      materialIndex  // materialIndex
  };

  return materialAsset;
}

void MaterialLoader::ExtractPBRParameters(aiMaterial *aiMat, MaterialMetadata &metadata)
{
  aiColor3D color;
  float value;
  int intValue;

  // 基础颜色因子（支持RGBA）
  if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, color) == AI_SUCCESS) {
    metadata.parameters["baseColorFactor"] = glm::vec4(color.r, color.g, color.b, 1.0f);
  }

  // 单独检查Alpha值（如果有）
  // 先尝试获取完整的baseColorFactor（包含alpha）
  aiColor4D color4D;
  if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, color4D) == AI_SUCCESS) {
    metadata.parameters["baseColorFactor"] = glm::vec4(color4D.r, color4D.g, color4D.b, color4D.a);
  }
  else if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, color) == AI_SUCCESS)
  {
    // 回退到RGB版本
    metadata.parameters["baseColorFactor"] = glm::vec4(color.r, color.g, color.b, 1.0f);
  }

  // 金属度因子
  if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, value) == AI_SUCCESS) {
    metadata.parameters["metallicFactor"] = value;
  }

  // 粗糙度因子
  if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
    metadata.parameters["roughnessFactor"] = value;
  }

  // 自发光因子
  if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
    metadata.parameters["emissiveFactor"] = glm::vec3(color.r, color.g, color.b);
  }

  // 透明度模式
  aiString alphaMode;
  if (aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
    std::string modeStr = alphaMode.C_Str();
    if (modeStr == "MASK") {
      metadata.alphaMode = AlphaMode::MASK;
    }
    else if (modeStr == "BLEND") {
      metadata.alphaMode = AlphaMode::BLEND;
    }
    else {
      metadata.alphaMode = AlphaMode::OPAQUE;
    }
  }

  // Alpha测试阈值（MASK模式使用）
  if (aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, value) == AI_SUCCESS) {
    metadata.alphaCutoff = value;
  }

  // 双面渲染标志
  int twoSided = 0;
  if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
    metadata.doubleSided = (twoSided != 0);
  }

  // 回退到传统材质参数（兼容非GLTF格式）
  if (metadata.parameters.find("baseColorFactor") == metadata.parameters.end()) {
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
      metadata.parameters["baseColorFactor"] = glm::vec4(color.r, color.g, color.b, 1.0f);
    }
  }
}

void MaterialLoader::ExtractTextureReferences(
    aiMaterial *aiMat,
    MaterialMetadata &metadata,
    const std::string &modelPath,
    const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures)
{

  aiString texturePath;

  // 基础颜色纹理
  if (aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &texturePath) ==
      AI_SUCCESS)
  {
    TextureAssetID texId = FindTextureAssetID(texturePath.C_Str(), loadedTextures);
    if (texId.IsValid()) {
      MaterialTextureSlot slot(texId);
      ExtractTextureTransform(aiMat,
                              AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE,
                              slot.scale,
                              slot.offset);
      metadata.textureSlots["baseColorTexture"] = slot;
    }
  }

  // 金属粗糙度纹理（也应当支持偏移，但很罕见）
  if (aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
                        &texturePath) == AI_SUCCESS)
  {
    TextureAssetID texId = FindTextureAssetID(texturePath.C_Str(), loadedTextures);
    if (texId.IsValid()) {
      metadata.textureSlots["metallicRoughnessTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 法线纹理
  if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = FindTextureAssetID(texturePath.C_Str(), loadedTextures);
    if (texId.IsValid()) {
      metadata.textureSlots["normalTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 自发光纹理
  if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = FindTextureAssetID(texturePath.C_Str(), loadedTextures);
    if (texId.IsValid()) {
      metadata.textureSlots["emissiveTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 环境光遮蔽纹理
  if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = FindTextureAssetID(texturePath.C_Str(), loadedTextures);
    if (texId.IsValid()) {
      metadata.textureSlots["occlusionTexture"] = MaterialTextureSlot(texId);
    }
  }
}

TextureAssetID MaterialLoader::FindTextureAssetID(
    const std::string &texturePath,
    const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures)
{

  // 处理嵌入式纹理路径
  std::string searchPath = texturePath;
  if (TextureLoader::IsEmbeddedTexturePath(texturePath)) {
    searchPath = texturePath;  // 直接使用嵌入式ID
  }

  // 在已加载的纹理中查找
  for (const auto &texture : loadedTextures) {
    if (texture && texture->metadata.sourcePath.find(searchPath) != std::string::npos) {
      return texture->id;
    }
  }

  LOG_WARN("Texture not found in loaded textures: " + texturePath);
  return TextureAssetID{};
}

void MaterialLoader::ExtractTextureTransform(aiMaterial *aiMat,
                                             aiTextureType textureType,
                                             unsigned int textureIndex,
                                             glm::vec2 &scale,
                                             glm::vec2 &offset)
{

  // 重置为默认值
  scale = glm::vec2(1.0f);
  offset = glm::vec2(0.0f);

  // 尝试提取GLTF的纹理变换参数
  aiUVTransform uvTransform;
  if (aiMat->Get(AI_MATKEY_UVTRANSFORM(textureType, textureIndex), uvTransform) == AI_SUCCESS) {
    scale.x = uvTransform.mScaling.x;
    scale.y = uvTransform.mScaling.y;
    offset.x = uvTransform.mTranslation.x;
    offset.y = uvTransform.mTranslation.y;
  }
}

std::string MaterialLoader::GenerateMaterialName(aiMaterial *aiMat,
                                                 uint32_t index,
                                                 const std::string &modelPath)
{

  aiString matName;
  if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS && strlen(matName.C_Str()) > 0) {
    return std::string(matName.C_Str());
  }

  // 从模型路径提取文件名作为前缀
  std::string fileName = modelPath.substr(modelPath.find_last_of("/\\") + 1);
  fileName = fileName.substr(0, fileName.find_last_of('.'));

  return fileName + "_Material_" + std::to_string(index);
}

std::shared_ptr<MaterialAsset> MaterialLoader::CreatePureColorMaterial(const std::string &name,
                                                                     const glm::vec3 &color)
{

  auto materialAsset = std::make_shared<MaterialAsset>();

  materialAsset->id = MaterialAssetID{UUIDGenerator::Generate(name.c_str())};
  materialAsset->metadata.name = name;
  materialAsset->metadata.templateName = PureColorMaterialTemplate::StaticType();

  // 设置纯色材质参数到通用参数存储
  materialAsset->metadata.parameters["baseColorFactor"] = glm::vec4(color, 1.0f);
  materialAsset->metadata.parameters["metallicFactor"] = 0.0f;
  materialAsset->metadata.parameters["roughnessFactor"] = 1.0f;
  materialAsset->metadata.parameters["emissiveFactor"] = glm::vec3(0.0f);

  materialAsset->metadata.alphaMode = AlphaMode::OPAQUE;
  materialAsset->metadata.doubleSided = false;

  return materialAsset;
}

}  // namespace mite
