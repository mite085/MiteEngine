#include "model_loader.h"
#include "basic_event/asset_event.h"
#include "material_loader.h"
#include "meshoptimizer.h"
#include "texture_loader.h"
#include <assimp/Importer.hpp>   // Assimp模型导入器
#include <assimp/pbrmaterial.h>  // AssimpPBR材质处理
#include <assimp/postprocess.h>  // Assimp后处理标志
#include <assimp/scene.h>

namespace mite {
ModelAssetID ModelLoader::LoadGLTFModel(ModelCache &modelCache,
                                        MaterialCache &materialCache,
                                        TextureCache &textureCache,
                                        const std::string &path,
                                        bool flipUVs,
                                        bool generateLODs,
                                        const std::vector<float> &lodLevels)
{
  // GLTF特化配置
  Assimp::Importer importer;
  unsigned int flags = GetAssimpImportFlags("gltf", flipUVs);

  // GLTF特定优化
  flags |= aiProcess_ImproveCacheLocality;  // GLTF已经优化过，可以跳过一些预处理

  const aiScene *scene = importer.ReadFile(
      path, flags);  // 无需释放，importer析构时会自动释放所有相关内存
  if (!scene || !scene->mRootNode) {
    LOG_ERROR("GLTF load failed: " + std::string(importer.GetErrorString()));
    return ModelAssetID{};
  }
  return LoadModelInternal(
      modelCache, materialCache, textureCache, scene, path, generateLODs, lodLevels);
}

ModelAssetID ModelLoader::LoadObjModel(ModelCache &modelCache,
                                       MaterialCache &materialCache,
                                       TextureCache &textureCache,
                                       const std::string &path,
                                       bool flipUVs,
                                       bool generateLODs,
                                       const std::vector<float> &lodLevels)
{
  // OBJ特化配置
  Assimp::Importer importer;
  unsigned int flags = GetAssimpImportFlags("obj", flipUVs);

  // OBJ特定处理
  flags |= aiProcess_OptimizeMeshes;  // OBJ通常需要网格优化

  const aiScene *scene = importer.ReadFile(path, flags);
  if (!scene || !scene->mRootNode) {
    LOG_ERROR("OBJ load failed: " + std::string(importer.GetErrorString()));
    return ModelAssetID{};
  }
  return LoadModelInternal(
      modelCache, materialCache, textureCache, scene, path, generateLODs, lodLevels);
}

ModelAssetID ModelLoader::LoadModel(ModelCache &modelCache,
                                    MaterialCache &materialCache,
                                    TextureCache &textureCache,
                                    const std::string &path,
                                    bool flipUVs,
                                    bool generateLODs,
                                    const std::vector<float> &lodLevels)
{
  // 通用模型加载
  Assimp::Importer importer;
  std::string extension = path.substr(path.find_last_of(".") + 1);
  unsigned int flags = GetAssimpImportFlags(extension, flipUVs);

  const aiScene *scene = importer.ReadFile(path, flags);
  if (!scene || !scene->mRootNode) {
    LOG_ERROR("Model load failed: " + std::string(importer.GetErrorString()));
    return ModelAssetID{};
  }
  return LoadModelInternal(
      modelCache, materialCache, textureCache, scene, path, generateLODs, lodLevels);
}

ModelAssetID ModelLoader::LoadModelInternal(ModelCache &modelCache,
                                            MaterialCache &materialCache,
                                            TextureCache &textureCache,
                                            const aiScene *scene,
                                            const std::string &path,
                                            bool generateLODs,
                                            const std::vector<float> &lodLevels)
{
  // 检查缓存
  ModelAssetID existingModelID = FindModelByPath(modelCache, path);
  if (existingModelID.IsValid()) {
    LOG_INFO("Model already cached: " + path);
    return existingModelID;
  }
  // 1. 加载材质（使用缓存系统）
  std::vector<MaterialAssetID> materialIDs = MaterialLoader::LoadMaterialsFromGLTF(
      materialCache, textureCache, scene, path);
  // 2. 处理所有子网格
  std::vector<MeshDataLODChain> subMeshData;
  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    MeshDataLODChain subMeshLodChain{
        scene->mMeshes[i]->mName.C_Str(), ProcessMesh(scene->mMeshes[i], scene), {}};
    // 生成多级LOD
    if (generateLODs) {
      // lodLevels = {1.0f, 0.5f, 0.25f, 0.1f}，
      // lodLevels[0] = 1.0f 为原始等级，不执行Simplify处理。
      // 故此处从1开始计数
      for (size_t lodLevel = 1; lodLevel < lodLevels.size(); lodLevel++) {
        MeshData simplifiedMesh = SimplifyMesh(subMeshLodChain.baseSection, lodLevels[lodLevel]);
        simplifiedMesh.lodLevel = static_cast<uint32_t>(lodLevel);
        subMeshLodChain.lodSections.push_back(simplifiedMesh);
      }
    }
    subMeshData.push_back(subMeshLodChain);
  }
  // 3. 创建模型资产
  auto modelAsset = std::make_shared<ModelAsset>();
  modelAsset->id = ModelAssetID{UUIDGenerator::Generate(path.c_str())};

  // 3.1. 设置元数据
  modelAsset->metadata.path = path;
  CalculateBoundingBox(
      subMeshData, modelAsset->metadata.boundingBoxMin, modelAsset->metadata.boundingBoxMax);

  // 3.2. 存储材质引用（MaterialAssetID）
  modelAsset->materialRefs = materialIDs;

  // 4. 创建ModelSourceData并构建MeshSectionLODChain
  std::shared_ptr<ModelSourceData> sourceData = CreateModelSourceData(modelAsset, subMeshData);

  // 5. 发布事件，委托RendererDevice创建GPU资源
  ModelLoadEvent event(sourceData, modelAsset);
  EventBus::Publish<ModelLoadEvent>(event);

  // 6. 存储到缓存
  if (modelCache.Store(modelAsset)) {
    LOG_INFO("Successfully loaded and cached model: " + path);
    return modelAsset->id;
  }
  else {
    LOG_ERROR("Failed to store model in cache: " + path);
    return ModelAssetID{};
  }
}

MeshData ModelLoader::ProcessMesh(const aiMesh *aiMesh, const aiScene *scene)
{
  MeshData subMesh;
  subMesh.layout = GenerateVertexLayout(aiMesh);

  // 1. 计算顶点数据总大小
  const uint32_t vertexSize = subMesh.layout.stride;
  const uint32_t vertexDataSize = aiMesh->mNumVertices * vertexSize;
  subMesh.vertexData.resize(vertexDataSize);

  // 2. 填充顶点数据
  uint8_t *vertexPtr = subMesh.vertexData.data();
  for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
    // 位置坐标
    glm::vec3 position(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
    memcpy(vertexPtr, &position, sizeof(glm::vec3));
    vertexPtr += sizeof(glm::vec3);

    // 法线
    if (aiMesh->HasNormals()) {
      glm::vec3 normal(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
      memcpy(vertexPtr, &normal, sizeof(glm::vec3));
      vertexPtr += sizeof(glm::vec3);
    }

    // 纹理坐标
    if (aiMesh->mTextureCoords[0]) {
      glm::vec2 uv(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
      memcpy(vertexPtr, &uv, sizeof(glm::vec2));
      vertexPtr += sizeof(glm::vec2);
    }

    // 切线/副切线
    if (aiMesh->HasTangentsAndBitangents()) {
      glm::vec3 tangent(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
      memcpy(vertexPtr, &tangent, sizeof(glm::vec3));
      vertexPtr += sizeof(glm::vec3);

      glm::vec3 bitangent(
          aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
      memcpy(vertexPtr, &bitangent, sizeof(glm::vec3));
      vertexPtr += sizeof(glm::vec3);
    }
  }

  // 3. 处理索引数据（相对于自己的顶点数据的相对偏移）
  subMesh.indices.reserve(aiMesh->mNumFaces * 3);
  for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
    const aiFace &face = aiMesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      subMesh.indices.push_back(face.mIndices[j]);
    }
  }

  // 4. 关联材质索引
  subMesh.materialIndex = aiMesh->mMaterialIndex;

  // 5. 计算子网格包围盒
  subMesh.boundingBoxMin = glm::vec3(FLT_MAX);
  subMesh.boundingBoxMax = glm::vec3(-FLT_MAX);
  const uint8_t *vPtr = subMesh.vertexData.data();
  for (size_t i = 0; i < subMesh.vertexData.size(); i += subMesh.layout.stride) {
    glm::vec3 position;
    memcpy(&position, vPtr + i, sizeof(glm::vec3));
    subMesh.boundingBoxMin = glm::min(subMesh.boundingBoxMin, position);
    subMesh.boundingBoxMax = glm::max(subMesh.boundingBoxMax, position);
  }

  return subMesh;
}

VertexLayout ModelLoader::GenerateVertexLayout(const aiMesh *aiMesh)
{
  VertexLayout layout;
  uint32_t offset = 0;

  // 按照 VertexAttribute 枚举顺序处理每个属性
  for (uint32_t i = 0; i < static_cast<uint32_t>(VertexAttribute::Count); ++i) {
    VertexAttribute attr = static_cast<VertexAttribute>(i);

    switch (attr) {
      case VertexAttribute::Position:
        // Position 是必需的
        layout.attributes.push_back(VertexAttribute::Position);
        offset += sizeof(glm::vec3);
        break;
      case VertexAttribute::Normal:
        if (aiMesh->HasNormals()) {
          layout.attributes.push_back(VertexAttribute::Normal);
          offset += sizeof(glm::vec3);
        }
        break;
      case VertexAttribute::TexCoord:
        if (aiMesh->mTextureCoords[0]) {
          layout.attributes.push_back(VertexAttribute::TexCoord);
          offset += sizeof(glm::vec2);
        }
        break;
      case VertexAttribute::Tangent:
        if (aiMesh->HasTangentsAndBitangents()) {
          layout.attributes.push_back(VertexAttribute::Tangent);
          offset += sizeof(glm::vec3);
        }
        break;
      case VertexAttribute::Bitangent:
        if (aiMesh->HasTangentsAndBitangents()) {
          layout.attributes.push_back(VertexAttribute::Bitangent);
          offset += sizeof(glm::vec3);
        }
        break;
      default:
        // 忽略未知属性
        break;
    }
  }
  layout.stride = offset;
  return layout;
}

MeshData ModelLoader::SimplifyMesh(const MeshData &originalMesh, float targetRatio)
{
  MeshData simplifiedMesh = originalMesh;

  if (originalMesh.indices.empty() || targetRatio >= 1.0f) {
    return simplifiedMesh;
  }
  // 准备meshoptimizer输入数据
  const size_t indexCount = originalMesh.indices.size();
  const size_t vertexCount = originalMesh.vertexData.size() / originalMesh.layout.stride;

  std::vector<unsigned int> indices = originalMesh.indices;

  // 首先进行顶点缓存优化
  meshopt_optimizeVertexCache(indices.data(), indices.data(), indexCount, vertexCount);

  // 计算目标索引数量
  const size_t targetIndexCount = static_cast<size_t>(indexCount * targetRatio);

  // 使用meshoptimizer进行网格简化（使用Sloppy确保顶点数量正常减少，而非靠target_error限制误差）
  std::vector<unsigned int> simplifiedIndices(indices.size());
  size_t simplifiedIndexCount = meshopt_simplifySloppy(
      simplifiedIndices.data(),
      indices.data(),
      indexCount,
      reinterpret_cast<const float *>(originalMesh.vertexData.data()),
      vertexCount,
      originalMesh.layout.stride,
      targetIndexCount,
      1);  // Sloppy无target_error输入

  // 调整简化后的索引数组大小
  simplifiedIndices.resize(simplifiedIndexCount);

  // 重新映射顶点数据
  std::vector<unsigned int> remap(vertexCount);
  size_t uniqueVertexCount = meshopt_optimizeVertexFetchRemap(
      remap.data(), simplifiedIndices.data(), simplifiedIndexCount, vertexCount);

  // 应用顶点重映射
  std::vector<uint8_t> simplifiedVertexData(uniqueVertexCount * originalMesh.layout.stride);
  meshopt_remapVertexBuffer(simplifiedVertexData.data(),
                            originalMesh.vertexData.data(),
                            vertexCount,
                            originalMesh.layout.stride,
                            remap.data());

  // 重映射索引（相对于自己的顶点数据的相对偏移）
  meshopt_remapIndexBuffer(
      simplifiedIndices.data(), simplifiedIndices.data(), simplifiedIndexCount, remap.data());

  // 更新简化后的网格数据
  simplifiedMesh.vertexData = std::move(simplifiedVertexData);
  simplifiedMesh.indices = std::move(simplifiedIndices);

  // 重新计算包围盒
  simplifiedMesh.boundingBoxMin = glm::vec3(FLT_MAX);
  simplifiedMesh.boundingBoxMax = glm::vec3(-FLT_MAX);
  const uint8_t *vPtr = simplifiedMesh.vertexData.data();
  for (size_t i = 0; i < simplifiedMesh.vertexData.size(); i += simplifiedMesh.layout.stride) {
    glm::vec3 position;
    memcpy(&position, vPtr + i, sizeof(glm::vec3));
    simplifiedMesh.boundingBoxMin = glm::min(simplifiedMesh.boundingBoxMin, position);
    simplifiedMesh.boundingBoxMax = glm::max(simplifiedMesh.boundingBoxMax, position);
  }

  return simplifiedMesh;
}

void ModelLoader::CalculateBoundingBox(const std::vector<MeshDataLODChain> &subMeshes,
                                       glm::vec3 &outMin,
                                       glm::vec3 &outMax)
{
  if (subMeshes.empty()) {
    outMin = outMax = glm::vec3(0.0f);
    return;
  }

  outMin = glm::vec3(FLT_MAX);
  outMax = glm::vec3(-FLT_MAX);

  for (const auto &subMesh : subMeshes) {
    outMin = glm::min(outMin, subMesh.baseSection.boundingBoxMin);
    outMax = glm::max(outMax, subMesh.baseSection.boundingBoxMax);
  }
}

std::shared_ptr<ModelSourceData> ModelLoader::CreateModelSourceData(
    std::shared_ptr<ModelAsset> model, const std::vector<MeshDataLODChain> &subMeshData)
{
  std::shared_ptr<ModelSourceData> sourceData = std::make_shared<ModelSourceData>();

  // 1. 准备合并所有子网格数据
  sourceData->path = model->metadata.path;
  sourceData->modelBboxMin = model->metadata.boundingBoxMin;
  sourceData->modelBboxMax = model->metadata.boundingBoxMax;

  // 获取layout类型
  if (!subMeshData.empty()) {
    sourceData->layout = subMeshData[0].baseSection.layout;
  }

  // 2. 合并顶点和索引数据
  size_t totalVertexBytes = 0;
  size_t totalIndices = 0;

  // 预计算总大小
  for (const auto &lodChain : subMeshData) {
    // 基础 LOD
    totalVertexBytes += lodChain.baseSection.vertexData.size();
    totalIndices += lodChain.baseSection.indices.size();

    // 其他 LOD 级别
    for (const auto &lodSection : lodChain.lodSections) {
      totalVertexBytes += lodSection.vertexData.size();
      totalIndices += lodSection.indices.size();
    }
  }

  // 预分配空间
  sourceData->mergedVertexData.reserve(totalVertexBytes);
  sourceData->mergedIndices.reserve(totalIndices);

  // 3. 实际合并数据并记录MeshSection
  uint32_t vertexOffset = 0;
  uint32_t indexOffset = 0;

  // 定义Lambda函数，兼顾合并顶点到sourceData、更新Offset、构建MeshSection三个功能
  auto MergeMeshDataToSourceData =
      [&sourceData, &vertexOffset, &indexOffset](const MeshData &meshData) -> MeshSection {
    // 添加顶点数据
    sourceData->mergedVertexData.insert(sourceData->mergedVertexData.end(),
                                        meshData.vertexData.begin(),
                                        meshData.vertexData.end());

    // 添加索引数据
    std::vector<uint32_t> adjustedIndices = meshData.indices;
    for (auto &index : adjustedIndices) {
      // 修正索引值偏移，将单个 MeshData 存储的相对偏移（相对于自己的顶点数据），
      // 修正为合并到 ModelSourceData 后的绝对偏移（相对于合并后的顶点数据）
      index += vertexOffset;
    }

    // 执行合并操作
    sourceData->mergedIndices.insert(
        sourceData->mergedIndices.end(), adjustedIndices.begin(), adjustedIndices.end());

    // 创建基础 MeshSection
    MeshSection meshSection = MeshSection{
        vertexOffset,  // 顶点偏移（以顶点计数为单位）
        indexOffset,   // 索引偏移（以索引计数为单位）
        static_cast<uint32_t>(meshData.vertexData.size() / meshData.layout.stride),
        static_cast<uint32_t>(meshData.indices.size()),
        meshData.boundingBoxMin,
        meshData.boundingBoxMax,
        meshData.materialIndex,
        meshData.lodLevel};

    // 更新偏移量
    vertexOffset += meshSection.vertexCount;
    indexOffset = static_cast<uint32_t>(sourceData->mergedIndices.size());

    return meshSection;
  };

  // 遍历MeshData并逐个执行合并操作，并构建MeshSection
  for (const MeshDataLODChain &meshLODChain : subMeshData) {
    MeshSectionLODChain sectionLODChain;

    // 传递Name
    sectionLODChain.name = meshLODChain.name;

    // 处理基础 LOD
    sectionLODChain.baseSection = MergeMeshDataToSourceData(meshLODChain.baseSection);

    // 处理其他 LOD 级别
    for (const MeshData &lodMeshData : meshLODChain.lodSections) {
      sectionLODChain.lodSections.push_back(MergeMeshDataToSourceData(lodMeshData));
    }

    // 由ModelAsset负责管理MeshSectionLODChain
    model->subMeshSection.push_back(sectionLODChain);
  }

  return sourceData;
}

unsigned int ModelLoader::GetAssimpImportFlags(const std::string &extension, bool flipUVs)
{
  unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
                       aiProcess_JoinIdenticalVertices;

  if (flipUVs) {
    flags |= aiProcess_FlipUVs;
  }

  // 格式特化配置
  if (extension == "gltf" || extension == "glb") {
    flags |= aiProcess_ImproveCacheLocality;
  }
  else if (extension == "obj") {
    flags |= aiProcess_OptimizeMeshes;
  }
  else if (extension == "fbx") {
    flags |= aiProcess_LimitBoneWeights;  // FBX通常有骨骼动画
  }

  return flags;
}
ModelAssetID ModelLoader::FindModelByPath(ModelCache &cache, const std::string &path)
{
  ModelAssetID searchId{UUIDGenerator::Generate(path.c_str())};
  auto model = cache.Get(searchId);
  return model ? searchId : ModelAssetID{};
}
};  // namespace mite