#ifndef MITE_RENDERER_MODEL
#define MITE_RENDERER_MODEL

#include "mesh.h"

namespace mite {
/**
 * 完整模型封装（包含多个子网格）
 * 职责：
 * - 管理模型所有子网格的生命周期
 * - 提供层级化绘制接口
 * 
 * TODO: Draw和DrawSubMesh修改为
 * 向RenderCommand发送Submit命令
 */
class Model {
 public:
  explicit Model(const ModelGPUHandle &handle);

  void Draw() const;
  void DrawSubMesh(size_t index) const;

  const size_t GetSubMeshCount() const
  {
    return subMeshes_.size();
  }
  const ModelGPUHandle &GetHandle() const
  {
    return handle_;
  }

 private:
  ModelGPUHandle handle_;
  std::vector<std::unique_ptr<Mesh>> subMeshes_;
};
};  // namespace mite

#endif
