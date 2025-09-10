#include "model_loader.h"
#include "basic_event/asset_event.h"
#include "meshoptimizer.h"
#include <assimp/Importer.hpp>   // Assimp模型导入器
#include <assimp/postprocess.h>  // Assimp后处理标志

namespace mite {
std::shared_ptr<ModelAsset> ModelLoader::LoadModel(const std::string &path,
                                                   bool flipUVs,
                                                   bool generateLODs,
                                                   const std::vector<float> &lodLevels)
{
  // 1. 配置Assimp导入器
  Assimp::Importer importer;
  unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
                       aiProcess_JoinIdenticalVertices | (flipUVs ? aiProcess_FlipUVs : 0);

  // 2. 加载模型文件
  const aiScene *scene = importer.ReadFile(path, flags);
  if (!scene || scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    LOG_ERROR("Assimp load failed: " + std::string(importer.GetErrorString()));
    return {};
  }

  std::shared_ptr<ModelAsset> model = std::make_shared<ModelAsset>();
  model->id = UUIDGenerator::Generate(path.c_str());  // 生成唯一ID
  model->metadata.path = path;
  model->metadata.materialPaths = ExtractMaterialPaths(scene);

  // 3. 处理所有子网格
  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    // 构建MeshLod链
    MeshDataLODChain subMeshLodChain{ProcessMesh(scene->mMeshes[i], scene), {}};

    // 4. 生成多级LOD
    if (generateLODs) {
      for (size_t lodLevel = 0; lodLevel < lodLevels.size(); lodLevel++) {
        // 跳过原始LOD级别
        if (lodLevels[lodLevel] < 1.0f) {
          MeshData simplifiedMesh = SimplifyMesh(subMeshLodChain.baseSection, lodLevels[lodLevel]);
          // LOD级别从1开始
          simplifiedMesh.lodLevel = static_cast<uint32_t>(lodLevel + 1);
          subMeshLodChain.lodSections.push_back(simplifiedMesh);
        }
      }
    }
    model->subMeshData.push_back(subMeshLodChain);
  }

  // 4. 计算模型包围盒
  CalculateBoundingBox(
      model->subMeshData, model->metadata.boundingBoxMin, model->metadata.boundingBoxMax);

  // 5. 构造RendererDevice可接收的ModelSourceData数据
  std::shared_ptr<ModelSourceData> sourceData = CreateModelSourceData(model);

  // 6. 发布事件，委托RendererDevice创建GPU资源
  ModelLoadEvent event(sourceData, model->handle);
  EventBus::Publish<ModelLoadEvent>(event);
  // model->handle = IRenderDevice::Current().CreateModel(rendererData);

  return model;
}

std::shared_ptr<ModelSourceData> ModelLoader::CreateModelSourceData(
    std::shared_ptr<ModelAsset> model)
{
  std::shared_ptr<ModelSourceData> sourceData = std::make_shared<ModelSourceData>();

  // 1. 准备合并所有子网格数据
  sourceData->path = model->metadata.path;
  sourceData->modelBboxMin = model->metadata.boundingBoxMin;
  sourceData->modelBboxMax = model->metadata.boundingBoxMax;
  if (!model->subMeshData.empty()) {
    sourceData->layout = model->subMeshData[0].baseSection.layout;
  }

  // 2. 合并顶点和索引数据
  size_t totalVertexBytes = 0;
  size_t totalIndices = 0;

  // 预计算总大小
  for (const auto &lodChain : model->subMeshData) {
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

  // 遍历MeshData并逐个处理，并构建MeshSection
  for (const MeshDataLODChain &meshLODChain : model->subMeshData) {
    MeshSectionLODChain sectionLODChain;

    // 处理基础 LOD
    sectionLODChain.baseSection = MergeMeshDataToSourceData(meshLODChain.baseSection);

    // 处理其他 LOD 级别
    for (const MeshData &lodMeshData : meshLODChain.lodSections) {
      sectionLODChain.lodSections.push_back(MergeMeshDataToSourceData(lodMeshData));
    }
    sourceData->sections.push_back(sectionLODChain);
  }

  return sourceData;
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

  layout.attributes.push_back(VertexAttribute::Position);
  offset += sizeof(glm::vec3);

  if (aiMesh->HasNormals()) {
    layout.attributes.push_back(VertexAttribute::Normal);
    offset += sizeof(glm::vec3);
  }

  if (aiMesh->mTextureCoords[0]) {
    layout.attributes.push_back(VertexAttribute::TexCoord);
    offset += sizeof(glm::vec2);
  }

  if (aiMesh->HasTangentsAndBitangents()) {
    layout.attributes.push_back(VertexAttribute::Tangent);
    offset += sizeof(glm::vec3);
    layout.attributes.push_back(VertexAttribute::Bitangent);
    offset += sizeof(glm::vec3);
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
  const float targetError = 1e-2f;  // 可接受的简化误差

  // 使用meshoptimizer进行网格简化
  std::vector<unsigned int> simplifiedIndices(indices.size());
  size_t simplifiedIndexCount = meshopt_simplify(
      simplifiedIndices.data(),
      indices.data(),
      indexCount,
      reinterpret_cast<const float *>(originalMesh.vertexData.data()),
      vertexCount,
      originalMesh.layout.stride,
      targetIndexCount,
      targetError);

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

std::vector<std::string> ModelLoader::ExtractMaterialPaths(const aiScene *scene)
{
  std::vector<std::string> materialPaths;
  materialPaths.reserve(scene->mNumMaterials);

  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    aiString path;
    if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
      materialPaths.emplace_back(path.C_Str());
    }
    else {
      materialPaths.emplace_back("");  // 空路径表示无材质
    }
  }

  return materialPaths;
}
};  // namespace mite