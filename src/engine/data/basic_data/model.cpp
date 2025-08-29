#include "model.h"

namespace mite {
Model::Model(std::shared_ptr<ModelGPUHandle> modelHandle)
    : modelHandle_(modelHandle), boundingBox_(modelHandle->bboxMin, modelHandle->bboxMax)
{
  if (!modelHandle_) {
    throw std::invalid_argument("Model handle cannot be null");
  }

  // 以下步骤目的主要为：从混在一起的std::vector<MeshSection>中，寻找所有原始LOD，以及基于该原始LOD生成的LOD变体

  // 首先建立 MeshSection* 到索引的映射，用于快速查找
  std::unordered_map<MeshSection *, size_t> sectionToIndex;
  for (size_t i = 0; i < modelHandle->subMeshes.size(); ++i) {
    sectionToIndex[&modelHandle->subMeshes[i]] = i;
  }

  // 按 LOD 级别分组：原始 LOD (level 0) 作为基础，其他作为 LOD 变体
  std::unordered_map<MeshSection *, std::vector<MeshSection *>> lodGroups;

  // 收集所有原始 LOD (level 0) 的 MeshSection
  for (auto &section : modelHandle->subMeshes) {
    if (section.lodLevel == 0) {
      lodGroups[&section].push_back(&section);
    }
  }

  // 然后处理所有 LOD 变体
  for (auto &section : modelHandle->subMeshes) {
    if (section.lodLevel > 0 && section.lodOriginPtr) {
      // 验证指针有效性
      if (sectionToIndex.find(section.lodOriginPtr) != sectionToIndex.end()) {
        // 按照指针进行LOD变体归类
        lodGroups[section.lodOriginPtr].push_back(&section); 
      }
      else {
        // 处理无效指针：作为独立 Mesh 处理
        LOG_WARN("Invalid lodOriginPtr found, treating as independent mesh");
        lodGroups[&section].push_back(&section);
      }
    }
  }

  // 基于每个原始 LOD 创建的分组，逐个构建 Mesh 对象
  for (auto &[baseSection, lodSections] : lodGroups) {
    // 按 LOD 级别排序
    std::sort(lodSections.begin(), lodSections.end(), [](MeshSection *a, MeshSection *b) {
      return a->lodLevel < b->lodLevel;
    });
    // 确保基础 LOD 是第一个
    if (lodSections.empty() || lodSections[0]->lodLevel != 0) {
      LOG_WARN("No base LOD found for mesh group, using first available");
      if (!lodSections.empty()) {
        lodSections.insert(lodSections.begin(), lodSections[0]);
      }
    }
    
    // 提取基础 LOD
    MeshSection baseSection = *lodSections[0];
    // 提取其他 LOD 级别（包含原始LOD）
    std::vector<MeshSection> otherLods;
    otherLods.reserve(lodSections.size());
    for (size_t i = 0; i < lodSections.size(); ++i) {
      otherLods.push_back(*lodSections[i]);
    }
    // 创建 Mesh 对象
    auto mesh = std::make_shared<Mesh>(modelHandle, baseSection, otherLods);
    subMeshes_.push_back(mesh);
  }


  LOG_INFO("Model loaded with {} submeshes, {} LOD sections total (Has LOD: {})",
           subMeshes_.size(),
           modelHandle->subMeshes.size(),
           hasLOD_);
}

size_t Model::GetSubMeshCount() const
{
  return subMeshes_.size();
}

const std::vector<std::shared_ptr<Mesh>> &Model::GetAllSubMeshes() const
{
  return subMeshes_;
}

std::shared_ptr<Mesh> Model::GetSubMesh(size_t index) const
{
  if (index >= subMeshes_.size()) {
    LOG_ERROR("Invalid submesh index: {}", index);
    return nullptr;
  }
  return subMeshes_[index];
}

std::vector<uint32_t> Model::GetSupportedLODLevels() const
{
  std::unordered_set<uint32_t> lodLevels;

  // 遍历所有SubMesh，获取Lod层级
  for (const auto &mesh : subMeshes_) {
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
  return boundingBox_;
}

const std::string Model::GetPath() const
{
  return modelHandle_->path;
}

bool Model::HasLOD() const
{
  return hasLOD_;
}
};  // namespace mite