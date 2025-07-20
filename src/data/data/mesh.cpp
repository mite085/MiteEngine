#include "mesh.h"

namespace mite {
Mesh::Mesh(const MeshGPUHandle &handle) : handle_(handle) {}

void Mesh::Draw() const
{
  // ÊÂ¼şÇı¶¯
  MeshDrawEvent event(handle_);
  EventBus::Get().Post(event);
}
};  // namespace mite