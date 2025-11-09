#include "shadow_map.h"

namespace mite {
ShadowMap::ShadowMap(const ShadowMapData &data) : m_Data(data)
{
  LOG_TRACE("ShadowMap created");
}
void ShadowMap::UpdateData(const ShadowMapData &data)
{
  if (m_Data.enabled != data.enabled) {
    LOG_TRACE("ShadowMap enabled state changed: {} -> {}", m_Data.enabled, data.enabled);
    m_NeedsUpdate = true;
  }
  m_Data = data;
}
bool ShadowMap::Validate() const
{
  return ValidateBaseParameters();
}
bool ShadowMap::ValidateBaseParameters() const
{
  return true;
}
}  // namespace mite
