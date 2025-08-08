#include "model.h"

namespace mite {
Model::Model(const std::vector<MeshGPUHandle> &handle)
{
  // 为每个子网格创建Mesh对象
  for (size_t i = 0; i < handle.size(); ++i) {
    subMeshes_.emplace_back(std::make_shared<Mesh>(std::make_shared<MeshGPUHandle>(handle[i])));
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
