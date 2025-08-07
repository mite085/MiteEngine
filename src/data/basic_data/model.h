#ifndef MITE_DATA_MODEL
#define MITE_DATA_MODEL

#include "mesh.h"

namespace mite {
/**
 * 完整模型封装（包含多个子网格）
 * 职责：
 * - 管理模型所有子网格的生命周期
 * - 提供层级化绘制接口
 */
class Model {
 public:
  explicit Model(const std::shared_ptr<ModelGPUHandle> handle);

  const size_t GetSubMeshCount() const
  {
    return subMeshes_.size();
  }
  const std::shared_ptr<ModelGPUHandle> GetHandle() const
  {
    return handle_;
  }
  std::shared_ptr<Mesh> GetMeshes(size_t count) const
  {
    if (count >= subMeshes_.size()) {
      LOG_ERROR("Invalid mesh count: {}", count);
      return nullptr;
    }
    return subMeshes_[count];
  }

 private:
  std::shared_ptr<ModelGPUHandle> handle_;
  std::vector<std::shared_ptr<Mesh>> subMeshes_;
};
};  // namespace mite

#endif
