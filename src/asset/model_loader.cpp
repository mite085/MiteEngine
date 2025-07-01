#include "model_loader.h"
#include <assimp/Importer.hpp>   // Assimp模型导入器
#include <assimp/postprocess.h>  // Assimp后处理标志
namespace mite {
ModelMetadata ModelLoader::LoadModel(const std::string &path, bool flipUVs)
{
  // 1. 配置Assimp导入器
  Assimp::Importer importer;
  unsigned int flags = aiProcess_Triangulate |       // 确保所有面都是三角形
                       aiProcess_GenNormals |        // 自动生成法线（如果模型没有）
                       aiProcess_CalcTangentSpace |  // 计算切线空间（用于法线贴图）
                       aiProcess_JoinIdenticalVertices |   // 合并重复顶点
                       (flipUVs ? aiProcess_FlipUVs : 0);  // 可选UV翻转

  // 2. 加载模型文件
  const aiScene *scene = importer.ReadFile(path, flags);
  if (!scene || scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    LOG_ERROR("Assimp加载失败: " + std::string(importer.GetErrorString()));
    return {};
  }

  ModelMetadata metadata;
  metadata.path = path;

  // 3. 处理所有子网格
  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    metadata.subMeshes.push_back(ProcessMesh(scene->mMeshes[i], scene));
  }

  // 4. 计算模型包围盒
  CalculateBoundingBox(metadata.subMeshes, metadata.boundingBoxMin, metadata.boundingBoxMax);

  return metadata;
}

SubMeshData ModelLoader::ProcessMesh(const aiMesh *aiMesh, const aiScene *scene)
{
  SubMeshData subMesh;
  subMesh.layout = GenerateVertexLayout(aiMesh);  // 生成顶点布局描述

  // 1. 计算顶点数据总大小
  const uint32_t vertexSize = subMesh.layout.stride;  // 单个顶点字节数
  const uint32_t vertexDataSize = aiMesh->mNumVertices * vertexSize;
  subMesh.vertexData.resize(vertexDataSize);

  // 2. 填充顶点数据（按布局描述打包）
  uint8_t *vertexPtr = subMesh.vertexData.data();
  for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
    // 位置坐标（必须存在）
    glm::vec3 position(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
    memcpy(vertexPtr, &position, sizeof(glm::vec3));
    vertexPtr += sizeof(glm::vec3);

    // 法线（如果存在）
    if (aiMesh->HasNormals()) {
      glm::vec3 normal(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
      memcpy(vertexPtr, &normal, sizeof(glm::vec3));
      vertexPtr += sizeof(glm::vec3);
    }

    // 纹理坐标（第一组UV）
    if (aiMesh->mTextureCoords[0]) {
      glm::vec2 uv(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
      memcpy(vertexPtr, &uv, sizeof(glm::vec2));
      vertexPtr += sizeof(glm::vec2);
    }

    // 切线/副切线（用于法线贴图）
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
  subMesh.indices.reserve(aiMesh->mNumFaces * 3);  // 预分配空间（三角形面）
  for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
    const aiFace &face = aiMesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      subMesh.indices.push_back(face.mIndices[j]);
    }
  }

  // 4. 关联材质索引
  subMesh.materialIndex = aiMesh->mMaterialIndex;

  return subMesh;
}

VertexLayout ModelLoader::GenerateVertexLayout(const aiMesh *aiMesh)
{
  VertexLayout layout;
  uint32_t offset = 0;

  // 位置（必须存在）
  layout.attributes.push_back(VertexAttribute::Position);
  offset += sizeof(glm::vec3);

  // 法线（如果存在）
  if (aiMesh->HasNormals()) {
    layout.attributes.push_back(VertexAttribute::Normal);
    offset += sizeof(glm::vec3);
  }

  // 纹理坐标（第一组UV）
  if (aiMesh->mTextureCoords[0]) {
    layout.attributes.push_back(VertexAttribute::TexCoord);
    offset += sizeof(glm::vec2);
  }

  // 切线/副切线（如果存在）
  if (aiMesh->HasTangentsAndBitangents()) {
    layout.attributes.push_back(VertexAttribute::Tangent);
    offset += sizeof(glm::vec3);

    layout.attributes.push_back(VertexAttribute::Bitangent);
    offset += sizeof(glm::vec3);
  }

  layout.stride = offset;  // 设置顶点总跨度
  return layout;
}

void ModelLoader::CalculateBoundingBox(const std::vector<SubMeshData> &subMeshes,
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
    const uint8_t *vertexPtr = subMesh.vertexData.data();
    for (size_t i = 0; i < subMesh.vertexData.size(); i += subMesh.layout.stride) {
      glm::vec3 position;
      memcpy(&position, vertexPtr + i, sizeof(glm::vec3));

      outMin = glm::min(outMin, position);
      outMax = glm::max(outMax, position);
    }
  }
}
};
