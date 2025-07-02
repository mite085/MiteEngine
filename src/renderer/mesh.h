#ifndef MITE_RENDERER_TEXTURE
#define MITE_RENDERER_TEXTURE

#include "asset_type.h"
#include "render_device.h"

namespace mite {
class Mesh {
 public:
  explicit Mesh(const MeshData &subMesh);
  ~Mesh();

  // 渲染网格（绑定VAO并调用glDrawElements）
  void Draw() const;

  // 获取顶点布局（供Shader使用）
  const VertexLayout &GetLayout() const
  {
    return m_Layout;
  }

 private:
  VertexLayout m_Layout;
};
};  // namespace mite

#endif