#include "model.h"

namespace mite {
Model::Model(ModelGPUHandle modelHandle, std::vector<MeshSectionLODChain> meshs,
             std::vector<std::shared_ptr<MaterialInstance>> materials)
    : m_Path(modelHandle.path),
      m_BoundingBox(modelHandle.bboxMin, modelHandle.bboxMax),
      m_Materials(materials) {
  // 基于每个原始 LOD 创建的分组，逐个构建 Mesh 对象
  for (MeshSectionLODChain &lodChain : meshs) {
    Mesh mesh = Mesh(modelHandle, lodChain);
    m_SubMeshes.push_back(mesh);
  }
}

size_t Model::GetSubMeshCount() const { return m_SubMeshes.size(); }

const std::vector<Mesh> &Model::GetAllSubMeshes() const { return m_SubMeshes; }

Mesh Model::GetSubMesh(size_t index) const {
  // 越界检查
  if (index < m_SubMeshes.size())
    return m_SubMeshes.at(index);
  else {
    LOG_ERROR(
        "Invalid getting submesh index: {}, Model path: {}, return empty mesh",
        index, m_Path);
    return Mesh();
  }
}

std::shared_ptr<MaterialInstance> Model::GetSubMaterial(size_t index) const {
  // 越界检查
  if (index < m_Materials.size()) {
    return m_Materials.at(index);
  } else {
    LOG_ERROR(
        "Invalid getting material index: {}, Model path: {}, return nullptr",
        index, m_Path);
    return nullptr;
  }
}

std::vector<uint32_t> Model::GetSupportedLODLevels() const {
  std::unordered_set<uint32_t> lodLevels;

  // 遍历所有SubMesh，获取Lod层级
  for (const auto &mesh : m_SubMeshes) {
    for (const auto &section : mesh.GetSubLODSections()) {
      lodLevels.insert(section.lodLevel);
    }
  }

  // 排序
  std::vector<uint32_t> result(lodLevels.begin(), lodLevels.end());
  std::sort(result.begin(), result.end());

  return result;
}

const std::pair<glm::vec3, glm::vec3> &Model::GetBoundingBox() const {
  return m_BoundingBox;
}

const std::string Model::GetPath() const { return m_Path; }

bool Model::HasLOD() const { return m_HasLOD; }
};  // namespace mite