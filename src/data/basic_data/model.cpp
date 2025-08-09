#include "model.h"

namespace mite {
Model::Model(std::shared_ptr<ModelGPUHandle> modelHandle)
    : modelHandle_(modelHandle), boundingBox_(modelHandle->bboxMin, modelHandle->bboxMax)
{
  // 为每个MeshSection创建对应的Mesh对象
  for (const auto &section : modelHandle->subMeshes) {
    subMeshes_.emplace_back(std::make_shared<Mesh>(modelHandle_, section));
  }
}

std::shared_ptr<Mesh> Model::GetSubMesh(size_t index) const
{
  if (index >= subMeshes_.size()) {
    LOG_ERROR("Invalid submesh index: {}", index);
    return nullptr;
  }
  return subMeshes_[index];
}
};
