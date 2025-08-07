#ifndef MITE_DATA_MESH
#define MITE_DATA_MESH

#include "headers/headers.h"
#include "basic_type/handle_type.h"

namespace mite {
/**
 * 子网格运行时封装
 * 职责：
 * - 维护单个子网格的GPU资源
 * - 提供最小化绘制接口
 */
class Mesh {
 public:
  Mesh(const std::shared_ptr<MeshGPUHandle> handle);

  uint32_t GetIndexCount() const
  {
    return handle_->indexCount;
  }
  std::shared_ptr<MeshGPUHandle> GetHandle() const
  {
    return handle_;
  }

 private:
  std::unique_ptr<MeshGPUHandle> handle_;
};

};  // namespace mite

#endif