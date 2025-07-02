#include "mesh.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后

namespace mite {
Mesh::Mesh(const MeshGPUHandle &handle) : handle_(handle) {}

void Mesh::Draw() const
{
  IRenderDevice::Current().BindMesh(handle_);
  IRenderDevice::Current().DrawIndexed(handle_.indexCount, 0);  // 从索引0开始绘制
}

};  // namespace mite