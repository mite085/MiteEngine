#ifndef MITE_RENDERER_MESH
#define MITE_RENDERER_MESH

#include "render_device.h"

namespace mite {
/**
 * 子网格运行时封装
 * 职责：
 * - 维护单个子网格的GPU资源
 * - 提供最小化绘制接口
 */
class Mesh {
 public:
  Mesh(const MeshGPUHandle &handle);

  void Draw() const;
  uint32_t GetIndexCount() const
  {
    return handle_.indexCount;
  }

 private:
  MeshGPUHandle handle_;
};
};  // namespace mite

#endif