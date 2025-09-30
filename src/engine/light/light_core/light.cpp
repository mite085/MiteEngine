#include "light.h"

namespace mite {
Light::Light(LightType type) : m_Type(type)
{
  // 设置类型特定的默认值（仅光学属性）
  switch (type) {
    case LightType::POINT:
      m_Properties.specific.point.radius = 10.0f;
      m_Properties.specific.point.falloff = 1.0f;
      break;
    case LightType::SPOT:
      m_Properties.specific.spot.innerAngle = 30.0f;
      m_Properties.specific.spot.outerAngle = 45.0f;
      m_Properties.specific.spot.blend = 0.5f;
      m_Properties.specific.spot.range = 10.0f;
      break;
    case LightType::DIRECTIONAL:
      m_Properties.specific.directional.irradiance = 1.0f;
      break;
    case LightType::AREA_RECT:
    case LightType::AREA_ELLIPSE:
      m_Properties.specific.area.size = glm::vec2(1.0f, 1.0f);
      m_Properties.specific.area.power = 100.0f;
      m_Properties.specific.area.shape = (type == LightType::AREA_RECT) ?
                                             AreaLightShape::RECTANGLE :
                                             AreaLightShape::ELLIPSE;
      break;
  }

  LOG_TRACE("Light created: type={}", static_cast<int>(type));
}

LightType Light::GetType() const
{
  return m_Type;
}
void Light::SetColor(const glm::vec3 &color)
{
  m_Properties.color = color;
}
const glm::vec3 &Light::GetColor() const
{
  return m_Properties.color;
}

void Light::SetIntensity(float intensity)
{
  m_Properties.intensity = intensity;
}
float Light::GetIntensity() const
{
  return m_Properties.intensity;
}

void Light::SetEnabled(bool enabled)
{
  m_Properties.enabled = enabled;
}
bool Light::IsEnabled() const
{
  return m_Properties.enabled;
}
LightProperties &Light::GetProperties()
{
  return m_Properties;
}
const LightProperties &Light::GetProperties() const
{
  return m_Properties;
}
void Light::SetProperties(const LightProperties &props)
{
  m_Properties = props;
}
void Light::SetShadowMap(ShadowMapPtr shadow)
{
  m_ShadowMap = shadow;
}
ShadowMapPtr Light::GetShadowMap() const
{
  return m_ShadowMap;
}
bool Light::IsCastingShadows() const
{
  return m_ShadowMap && m_ShadowMap->GetData().enabled;
}
ShadowMapData Light::PrepareShadowData(const Transform &worldTransform,
                                       const Transform &cameraView,
                                       const glm::mat4 &cameraProj) const
{
  if (m_ShadowMap && m_Properties.enabled) {
    return m_ShadowMap->PrepareShadowData(worldTransform, cameraView, cameraProj);
  }
  return ShadowMapData();
}
bool Light::Validate() const
{
  return ValidateBaseParameters();
}
bool Light::ValidateBaseParameters() const
{
  if (m_Properties.intensity < 0.0f) {
    LOG_ERROR("Light intensity cannot be negative: {}", m_Properties.intensity);
    return false;
  }

  if (glm::length(m_Properties.color) < 0.001f) {
    LOG_WARN("Light color is nearly black, may not be visible");
  }

  return true;
}
}  // namespace mite