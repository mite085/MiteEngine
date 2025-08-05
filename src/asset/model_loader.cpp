#include "model_loader.h"
#include <assimp/Importer.hpp>   // Assimp模型导入器
#include <assimp/postprocess.h>  // Assimp后处理标志
namespace mite {
std::shared_ptr<ModelAsset> ModelLoader::LoadModel(const std::string &path, bool flipUVs)
{
  // 1. 配置Assimp导入器
  Assimp::Importer importer;
  unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
                       aiProcess_JoinIdenticalVertices | (flipUVs ? aiProcess_FlipUVs : 0);

  // 2. 加载模型文件
  const aiScene *scene = importer.ReadFile(path, flags);
  if (!scene || scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    LOG_ERROR("Assimp加载失败: " + std::string(importer.GetErrorString()));
    return {};
  }

  std::shared_ptr<ModelAsset> model = std::make_shared<ModelAsset>();
  model->id = UUIDGenerator::Generate(path.c_str());  // 生成唯一ID
  model->metadata.path = path;
  model->metadata.materialPaths = ExtractMaterialPaths(scene);

  // 3. 处理所有子网格
  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    model->subMeshes.push_back(ProcessMesh(scene->mMeshes[i], scene));
  }

  // 4. 计算模型包围盒
  CalculateBoundingBox(
      model->subMeshes, model->metadata.boundingBoxMin, model->metadata.boundingBoxMax);

  // 5. (该步骤移交给RendererDevice处理)
  //    转换 Asset 模块数据为 Renderer 模块的 ModelSourceData
  //ModelSourceData rendererData;
  //rendererData.modelBboxMin = model->metadata.boundingBoxMin;
  //rendererData.modelBboxMax = model->metadata.boundingBoxMax;

  //for (const auto &subMesh : model->subMeshes) {
  //  rendererData.subMeshes.push_back(
  //      {subMesh.vertexData.data(),
  //       subMesh.indices.data(),
  //       static_cast<uint32_t>(subMesh.vertexData.size() / subMesh.layout.stride),
  //       static_cast<uint32_t>(subMesh.indices.size()),
  //       subMesh.layout,
  //       subMesh.boundingBoxMin,
  //       subMesh.boundingBoxMax});
  //}

  // 6. 发布事件，委托RendererDevice创建GPU资源
  ModelLoadEvent event(model);
  EventBus::Get().Post(event);
  // model->handle = IRenderDevice::Current().CreateModel(rendererData);

  return model;
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

  // 3. 处理索引数据
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

void ModelLoader::CalculateBoundingBox(const std::vector<MeshData> &subMeshes,
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
    outMin = glm::min(outMin, subMesh.boundingBoxMin);
    outMax = glm::max(outMax, subMesh.boundingBoxMax);
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