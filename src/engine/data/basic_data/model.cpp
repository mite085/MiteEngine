#include "model.h"

namespace mite {
Model::Model(std::shared_ptr<ModelGPUHandle> modelHandle)
    : m_ModelHandle(modelHandle), m_BoundingBox(modelHandle->bboxMin, modelHandle->bboxMax)
{
  if (!m_ModelHandle) {
    throw std::invalid_argument("Model handle cannot be null");
  }

  // 基于每个原始 LOD 创建的分组，逐个构建 Mesh 对象
  for (MeshSectionLODChain &lodChain : m_ModelHandle->subMeshes) {
    auto mesh = std::make_shared<Mesh>(modelHandle, lodChain);
    m_SubMeshes.push_back(mesh);
  }
}

size_t Model::GetSubMeshCount() const
{
  return m_SubMeshes.size();
}

const std::vector<std::shared_ptr<Mesh>> &Model::GetAllSubMeshes() const
{
  return m_SubMeshes;
}

std::shared_ptr<Mesh> Model::GetSubMesh(size_t index) const
{
  if (index >= m_SubMeshes.size()) {
    LOG_ERROR("Invalid submesh index: {}", index);
    return nullptr;
  }
  return m_SubMeshes[index];
}

std::vector<uint32_t> Model::GetSupportedLODLevels() const
{
  std::unordered_set<uint32_t> lodLevels;

  // 遍历所有SubMesh，获取Lod层级
  for (const auto &mesh : m_SubMeshes) {
    for (const auto &section : mesh->GetAllLODSections()) {
      lodLevels.insert(section.lodLevel);
    }
  }

  // 排序
  std::vector<uint32_t> result(lodLevels.begin(), lodLevels.end());
  std::sort(result.begin(), result.end());

  return result;
}

const std::pair<glm::vec3, glm::vec3> &Model::GetBoundingBox() const
{
  return m_BoundingBox;
}

const std::string Model::GetPath() const
{
  return m_ModelHandle->path;
}

bool Model::HasLOD() const
{
  return m_HasLOD;
}
};  // namespace mite