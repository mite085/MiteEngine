#include "mesh.h"

namespace mite {
Mesh::Mesh(std::shared_ptr<ModelGPUHandle> modelHandle, const MeshSection &section)
    : modelHandle_(modelHandle), section_(section)
{
  if (!modelHandle_) {
    throw std::invalid_argument("Model handle cannot be null");
  }

  if (section_.vertexCount == 0 || section_.indexCount == 0) {
    throw std::invalid_argument("Invalid mesh section: vertex or index count is zero");
  }
}
};  // namespace mite