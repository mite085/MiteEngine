#include "shadow_map.h"

namespace mite {
ShadowMap::ShadowMap(const ShadowMapData &data) : m_Data(data)
{
  LOG_TRACE("ShadowMap created with quality: {}", static_cast<int>(data.quality));
}
void ShadowMap::UpdateData(const ShadowMapData &data)
{
  if (m_Data.enabled != data.enabled) {
    LOG_TRACE("ShadowMap enabled state changed: {} -> {}", m_Data.enabled, data.enabled);
    m_NeedsUpdate = true;
  }

  if (m_Data.quality != data.quality) {
    LOG_TRACE("ShadowMap quality changed: {} -> {}",
              static_cast<int>(m_Data.quality),
              static_cast<int>(data.quality));
    m_NeedsUpdate = true;
  }

  if (m_Data.bias != data.bias) {
    LOG_TRACE("ShadowMap bias changed: {} -> {}", m_Data.bias, data.bias);
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
  if (m_Data.bias < 0.0f) {
    LOG_ERROR("Shadow bias cannot be negative: {}", m_Data.bias);
    return false;
  }

  if (m_Data.normalBias < 0.0f) {
    LOG_ERROR("Shadow normal bias cannot be negative: {}", m_Data.normalBias);
    return false;
  }

  return true;
}
}  // namespace mite
