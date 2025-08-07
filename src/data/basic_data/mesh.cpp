#include "mesh.h"

namespace mite {
Mesh::Mesh(const std::shared_ptr<MeshGPUHandle> handle) : handle_(handle)
{
  if (handle->vertexArray == 0 || handle->indexBuffer == 0) {
    throw std::invalid_argument("Invalid MeshGPUHandle: vertexArray or indexBuffer is null");
  }
  if (handle->vertexCount == 0 || handle->indexCount == 0) {
    throw std::invalid_argument("Invalid MeshGPUHandle: vertexCount or indexCount is zero");
  }
}
};  // namespace mite