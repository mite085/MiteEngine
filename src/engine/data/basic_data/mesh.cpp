#include "mesh.h"

namespace mite {
Mesh::Mesh(std::shared_ptr<ModelGPUHandle> modelHandle,
           const MeshSection &baseSection,
           const std::vector<MeshSection> &lodSections)
    : modelHandle_(modelHandle), baseSection_(baseSection), lodSections_(lodSections)
{
  if (!modelHandle_) {
    throw std::invalid_argument("Model handle cannot be null");
  }

  if (baseSection_.vertexCount == 0 || baseSection_.indexCount == 0) {
    throw std::invalid_argument("Invalid mesh section: vertex or index count is zero");
  }

  // 确保lodSections包含基础LOD级别
  if (lodSections_.empty()) {
    lodSections_.push_back(baseSection_);
  }
  else {
    // 检查第一个LOD级别是否是基础级别
    if (lodSections_[0].lodLevel != 0) {
      lodSections_.insert(lodSections_.begin(), baseSection_);
    }
  }
}

uint32_t Mesh::GetVertexCount(uint32_t lodLevel) const
{
  for (const auto &section : lodSections_) {
    if (section.lodLevel == lodLevel) {
      return section.vertexCount;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return baseSection_.vertexCount;
}

uint32_t Mesh::GetIndexCount(uint32_t lodLevel) const
{
  for (const auto &section : lodSections_) {
    if (section.lodLevel == lodLevel) {
      return section.indexCount;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return baseSection_.indexCount;
}

uint32_t Mesh::GetIndexOffset(uint32_t lodLevel) const
{
  for (const auto &section : lodSections_) {
    if (section.lodLevel == lodLevel) {
      return section.indexOffset;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的数据
  return baseSection_.indexOffset;
}

const MeshSection &Mesh::GetSection(uint32_t lodLevel) const
{
  for (const auto &section : lodSections_) {
    if (section.lodLevel == lodLevel) {
      return section;
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别
  return baseSection_;
}

const MeshSection &Mesh::GetBaseSection() const
{
  return baseSection_;
}

const std::vector<MeshSection> &Mesh::GetAllLODSections() const
{
  return lodSections_;
}

uint32_t Mesh::GetLODCount() const
{
  return static_cast<uint32_t>(lodSections_.size());
}

std::shared_ptr<ModelGPUHandle> Mesh::GetModelHandle() const
{
  return modelHandle_;
}

const std::pair<glm::vec3, glm::vec3> Mesh::GetBoundingBox(uint32_t lodLevel) const
{
  for (const auto &section : lodSections_) {
    if (section.lodLevel == lodLevel) {
      return {section.bboxMin, section.bboxMax};
    }
  }
  // 如果找不到指定LOD级别，返回基础LOD级别的包围盒
  return {baseSection_.bboxMin, baseSection_.bboxMax};
}
/**
 * 获取材质索引
 */
uint32_t Mesh::GetMaterialIndex() const
{
  return baseSection_.materialIndex;
}
};  // namespace mite