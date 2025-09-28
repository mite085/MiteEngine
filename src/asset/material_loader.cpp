#include "material_loader.h"
#include "material_templates/material_template_gltf_pbr.h"
#include "material_templates/material_template_pure_color.h"
#include "texture_loader.h"
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/scene.h>

namespace mite {
std::vector<MaterialAssetID> MaterialLoader::LoadMaterialsFromGLTF(MaterialCache &materialCache,
                                                                   TextureCache &textureCache,
                                                                   const aiScene *scene,
                                                                   const std::string &modelPath)
{
  std::vector<MaterialAssetID> materialIDs;

  if (!scene) {
    LOG_ERROR("Null scene provided for material loading");
    return materialIDs;
  }

  // 处理场景中的所有材质
  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    aiMaterial *aiMat = scene->mMaterials[i];
    MaterialAssetID materialID = ProcessGLTFMaterial(
        materialCache, textureCache, aiMat, i, modelPath, scene);

    if (materialID.IsValid()) {
      materialIDs.push_back(materialID);
      // 获取材质资产用于事件发布
      std::shared_ptr<MaterialAsset> materialAsset = materialCache.Get(materialID);
      if (materialAsset) {
        LOG_INFO("Successfully loaded material: " + materialAsset->metadata.name);

        // 创建纹理解析器回调函数（TODO: 这里已经能正常获取到TextureInstance了，无需使用回调函数）
        auto textureResolver = [&textureCache](const TextureAssetID &id) -> TextureInstance {
          std::shared_ptr<TextureAsset> texture = textureCache.Get(id);
          return texture ? texture->instance : TextureInstance{};
        };

        // 生成MaterialSourceData并发布事件
        MaterialSourceData sourceData = materialAsset->metadata.generateSourceData(
            textureResolver);
        EventBus::Publish(MaterialLoadedEvent(sourceData, materialAsset));
      }
    }
    else {
      LOG_WARN("Failed to process material at index: " + std::to_string(i));
      // 创建默认材质作为回退
      MaterialAssetID fallbackID = CreatePureColorMaterial(
          materialCache, "Fallback_Material_" + std::to_string(i));
      if (fallbackID.IsValid()) {
        materialIDs.push_back(fallbackID);

        auto fallbackMaterial = materialCache.Get(fallbackID);
        if (fallbackMaterial) {
          auto textureResolver = [](const TextureAssetID &) { return TextureInstance{}; };
          MaterialSourceData sourceData = fallbackMaterial->metadata.generateSourceData(
              textureResolver);
          EventBus::Publish(MaterialLoadedEvent(sourceData, fallbackMaterial));
        }
      }
    }
  }
  // 如果没有材质，创建默认材质
  if (materialIDs.empty()) {
    LOG_INFO("No materials found in scene, creating default material");
    MaterialAssetID defaultID = CreatePureColorMaterial(materialCache, "Default_Material");
    if (defaultID.IsValid()) {
      materialIDs.push_back(defaultID);

      auto defaultMaterial = materialCache.Get(defaultID);
      if (defaultMaterial) {
        auto textureResolver = [](const TextureAssetID &) { return TextureInstance{}; };
        MaterialSourceData sourceData = defaultMaterial->metadata.generateSourceData(
            textureResolver);
        EventBus::Publish(MaterialLoadedEvent(sourceData, defaultMaterial));
      }
    }
  }
  return materialIDs;
}

MaterialAssetID MaterialLoader::ProcessGLTFMaterial(MaterialCache &materialCache,
                                                    TextureCache &textureCache,
                                                    aiMaterial *aiMat,
                                                    uint32_t materialIndex,
                                                    const std::string &modelPath,
                                                    const aiScene *scene)
{
  if (!aiMat) {
    LOG_ERROR("Null aiMaterial provided");
    return MaterialAssetID{};
  }

  // 生成材质唯一标识
  std::string materialName = GenerateMaterialName(aiMat, materialIndex, modelPath);
  std::string materialKey = modelPath + "::" + materialName;
  MaterialAssetID materialID{UUIDGenerator::Generate(materialKey.c_str())};

  // 检查缓存中是否已存在
  auto existingMaterial = materialCache.Get(materialID);
  if (existingMaterial) {
    LOG_INFO("Material already cached: " + materialKey);
    return materialID;
  }

  // 创建材质资产
  auto materialAsset = std::make_shared<MaterialAsset>();
  materialAsset->id = materialID;

  // 设置基础元数据
  materialAsset->metadata.sourcePath = modelPath;
  materialAsset->metadata.name = materialName;
  materialAsset->metadata.templateName = GLTFPBRMaterialTemplate::StaticType();  // 对应的材质模板

  // 提取GLTF PBR参数
  ExtractPBRParameters(aiMat, materialAsset->metadata);

  // 提取并创建纹理引用
  ExtractAndCreateTextureReferences(
      textureCache, aiMat, materialAsset->metadata, modelPath, scene);

  // 设置GLTF源信息
  materialAsset->metadata.sourceInfo = MaterialMetadata::GLTFSourceInfo{
      modelPath,     // gltfFilePath
      materialIndex  // materialIndex
  };

  // 存储到缓存
  if (materialCache.Store(materialAsset)) {
    return materialID;
  }
  else {
    LOG_ERROR("Failed to store material in cache: " + materialKey);
    return MaterialAssetID{};
  }
}

void MaterialLoader::ExtractPBRParameters(aiMaterial *aiMat, MaterialMetadata &metadata)
{
  aiColor3D color;
  float value;

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

void MaterialLoader::ExtractAndCreateTextureReferences(TextureCache &textureCache,
                                                       aiMaterial *aiMat,
                                                       MaterialMetadata &metadata,
                                                       const std::string &modelPath,
                                                       const aiScene *scene)
{
  aiString texturePath;

  // 基础颜色纹理
  if (aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &texturePath) ==
      AI_SUCCESS)
  {
    TextureAssetID texId = CreateOrGetTextureAssetID(
        textureCache, texturePath.C_Str(), modelPath, scene);
    if (texId.IsValid()) {
      MaterialTextureSlot slot(texId);
      ExtractTextureTransform(
          aiMat, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, slot.scale, slot.offset);
      metadata.textureSlots["baseColorTexture"] = slot;
    }
  }

  // 金属粗糙度纹理（也应当支持偏移，但很罕见）
  if (aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
                        &texturePath) == AI_SUCCESS)
  {
    TextureAssetID texId = CreateOrGetTextureAssetID(
        textureCache, texturePath.C_Str(), modelPath, scene);
    if (texId.IsValid()) {
      metadata.textureSlots["metallicRoughnessTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 法线纹理
  if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = CreateOrGetTextureAssetID(
        textureCache, texturePath.C_Str(), modelPath, scene);
    if (texId.IsValid()) {
      metadata.textureSlots["normalTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 自发光纹理
  if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = CreateOrGetTextureAssetID(
        textureCache, texturePath.C_Str(), modelPath, scene);
    if (texId.IsValid()) {
      metadata.textureSlots["emissiveTexture"] = MaterialTextureSlot(texId);
    }
  }

  // 环境光遮蔽纹理
  if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {
    TextureAssetID texId = CreateOrGetTextureAssetID(
        textureCache, texturePath.C_Str(), modelPath, scene);
    if (texId.IsValid()) {
      metadata.textureSlots["occlusionTexture"] = MaterialTextureSlot(texId);
    }
  }
}

TextureAssetID MaterialLoader::CreateOrGetTextureAssetID(TextureCache &textureCache,
                                                         const std::string &texturePath,
                                                         const std::string &modelPath,
                                                         const aiScene *scene)
{
  if (texturePath.empty()) {
    LOG_WARN("Empty texture path provided");
    return TextureAssetID{};
  }

  // 解析纹理完整路径（处理相对路径）
  std::string resolvedPath = ResolveTexturePath(texturePath, modelPath);

  // 判断嵌入式/外部纹理
  if (TextureLoader::IsEmbeddedTexturePath(texturePath)) {
    // 处理嵌入式纹理
    if (scene) {
      std::string embeddedId = texturePath.substr(1);  // 去掉'*'
      int textureIndex = std::stoi(embeddedId);

      if (textureIndex >= 0 && textureIndex < (int)scene->mNumTextures) {
        const aiTexture *aiTex = scene->mTextures[textureIndex];
        return TextureLoader::LoadEmbeddedTexture(textureCache, texturePath, modelPath, aiTex);
      }
    }
  }
  else {
    // 处理外部纹理文件
    return TextureLoader::LoadTexture(textureCache, resolvedPath);
  }
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

MaterialAssetID MaterialLoader::CreatePureColorMaterial(MaterialCache &materialCache,
                                                        const std::string &name,
                                                        const glm::vec3 &color)
{
  std::string materialKey = PureColorMaterialTemplate::StaticType() + "_" + name;
  MaterialAssetID materialID{UUIDGenerator::Generate(materialKey.c_str())};

  // 检查缓存中是否已存在
  auto existingMaterial = materialCache.Get(materialID);
  if (existingMaterial) {
    return materialID;
  }

  auto materialAsset = std::make_shared<MaterialAsset>();

  materialAsset->id = materialID;
  materialAsset->metadata.name = name;
  materialAsset->metadata.templateName = PureColorMaterialTemplate::StaticType();

  // 设置纯色材质参数到通用参数存储
  materialAsset->metadata.parameters["baseColorFactor"] = glm::vec4(color, 1.0f);
  materialAsset->metadata.parameters["metallicFactor"] = 0.0f;
  materialAsset->metadata.parameters["roughnessFactor"] = 1.0f;
  materialAsset->metadata.parameters["emissiveFactor"] = glm::vec3(0.0f);

  materialAsset->metadata.alphaMode = AlphaMode::OPAQUE;
  materialAsset->metadata.doubleSided = false;

  // 存储到缓存
  if (materialCache.Store(materialAsset)) {
    return materialID;
  }
  else {
    LOG_ERROR("Failed to store pure color material in cache: " + name);
    return MaterialAssetID{};
  }
}

std::string MaterialLoader::ResolveTexturePath(const std::string &texturePath,
                                               const std::string &modelPath)
{
  if (texturePath.empty())
    return texturePath;
  // 如果是绝对路径或嵌入式纹理，直接返回
  if (texturePath[0] == '/' || texturePath[0] == '\\' || texturePath[0] == '*') {
    return texturePath;
  }
  // 处理相对路径：相对于模型文件所在目录
  std::string modelDir = modelPath.substr(0, modelPath.find_last_of("/\\") + 1);
  return modelDir + texturePath;
}

}  // namespace mite