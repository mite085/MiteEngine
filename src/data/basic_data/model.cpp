#include "model.h"

namespace mite {
Model::Model(const ModelGPUHandle &handle)
    : handle_(handle)
{
  // 为每个子网格创建Mesh对象
  for (size_t i = 0; i < handle.subMeshes.size(); ++i) {
    subMeshes_.emplace_back(std::make_unique<Mesh>(handle.subMeshes[i]));
  }
}

//void Model::Draw() const
//{
//  for (const auto &subMesh : subMeshes_) {
//    subMesh->Draw();
//  }
//}
//
//void Model::DrawSubMesh(size_t index) const
//{
//  if (index < subMeshes_.size()) {
//    subMeshes_[index]->Draw();
//  }
//}
};
