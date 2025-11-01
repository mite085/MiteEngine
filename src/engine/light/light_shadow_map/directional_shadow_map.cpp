#include "directional_shadow_map.h"

namespace mite {
DirectionalShadowMap::DirectionalShadowMap(const ShadowMapData &data)
    : ShadowMap(data)
{
}
ShadowMapData DirectionalShadowMap::PrepareShadowData(const Transform &lightWorldTransform,
                                                      const Transform &cameraView,
                                                      const glm::mat4 &cameraProj)
{
  return ShadowMapData();
}
size_t DirectionalShadowMap::GetShadowMatrixCount() const
{
  return size_t();
}
glm::mat4 DirectionalShadowMap::GetShadowMatrix(size_t index) const
{
  return glm::mat4();
}
bool DirectionalShadowMap::NeedsUpdate() const
{
  return false;
}
void DirectionalShadowMap::MarkUpdated() {}
std::string DirectionalShadowMap::GetShadowTypeName() const
{
  return "DirectionalShadowMap";
}
}  // namespace mite
