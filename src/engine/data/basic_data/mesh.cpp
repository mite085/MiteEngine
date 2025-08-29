#include "mesh.h"

namespace mite {
Mesh::Mesh(std::shared_ptr<ModelGPUHandle> modelHandle, const MeshSectionLODChain &lodChain)
    : modelHandle_(modelHandle), lodChain_(lodChain)
{
  if (!modelHandle_) {
    throw std::invalid_argument("Model handle cannot be null");
  }

  // 检查是否为空Mesh
  if (lodChain_.baseSection.vertexCount == 0 || lodChain_.baseSection.indexCount == 0) {
    throw std::invalid_argument("Invalid mesh section: vertex or index count is zero");
  }

}

uint32_t Mesh::GetVertexCount(uint32_t lodLevel) const
{
  for (const auto &section : lodChain_.lodSections) {
    if (section.lodLevel == lodLevel) {
      return section.vertexCount;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return lodChain_.baseSection.vertexCount;
}

uint32_t Mesh::GetIndexCount(uint32_t lodLevel) const
{
  for (const auto &section : lodChain_.lodSections) {
    if (section.lodLevel == lodLevel) {
      return section.indexCount;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return lodChain_.baseSection.indexCount;
}

uint32_t Mesh::GetIndexOffset(uint32_t lodLevel) const
{
  for (const auto &section : lodChain_.lodSections) {
    if (section.lodLevel == lodLevel) {
      return section.indexOffset;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return lodChain_.baseSection.indexOffset;
}

const MeshSection &Mesh::GetSection(uint32_t lodLevel) const
{
  for (const auto &section : lodChain_.lodSections) {
    if (section.lodLevel == lodLevel) {
      return section;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别
  return lodChain_.baseSection;
}

const MeshSection &Mesh::GetBaseSection() const
{
  return lodChain_.baseSection;
}

const std::vector<MeshSection> &Mesh::GetAllLODSections() const
{
  return lodChain_.lodSections;
}

uint32_t Mesh::GetLODCount() const
{
  return static_cast<uint32_t>(lodChain_.lodSections.size());
}

std::shared_ptr<ModelGPUHandle> Mesh::GetModelHandle() const
{
  return modelHandle_;
}

const std::pair<glm::vec3, glm::vec3> Mesh::GetBoundingBox(uint32_t lodLevel) const
{
  for (const auto &section : lodChain_.lodSections) {
    if (section.lodLevel == lodLevel) {
      return {section.bboxMin, section.bboxMax};
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的包围盒
  return {lodChain_.baseSection.bboxMin, lodChain_.baseSection.bboxMax};
}
/**
 * 获取材质索引
 */
uint32_t Mesh::GetMaterialIndex() const
{
  return lodChain_.baseSection.materialIndex;
}
};  // namespace mite