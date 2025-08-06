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
  explicit Model(const ModelGPUHandle &handle);

  const size_t GetSubMeshCount() const
  {
    return subMeshes_.size();
  }
  const ModelGPUHandle &GetHandle() const
  {
    return handle_;
  }
  Mesh* GetMeshes(size_t count) const
  {
    if (count >= subMeshes_.size()) {
      LOG_ERROR("Invalid mesh count: {}", count);
      return nullptr;
    }
    return subMeshes_[count].get();
  }

 private:
  ModelGPUHandle handle_;
  std::vector<std::unique_ptr<Mesh>> subMeshes_;
};
};  // namespace mite

#endif
