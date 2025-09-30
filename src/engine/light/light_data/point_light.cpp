#include "point_light.h"

namespace mite {

PointLight::PointLight() : Light(LightType::POINT)
{
  // 点光源的默认参数已在Light基类构造函数中设置
  LOG_TRACE("PointLight created with radius: {}, intensity: {}",
            m_Properties.specific.point.radius,
            m_Properties.intensity);
}

std::string PointLight::GetLightTypeName() const
{
  return "PointLight";
}

GPULightData PointLight::PrepareGPULightData(const Transform &worldTransform) const
{
  if (!m_Properties.enabled) {
    LOG_WARN("Preparing GPU data for disabled point light");
  }

  // 使用基类的GPULightData构造函数，传入点光源类型
  GPULightData gpuData(m_Properties, worldTransform, LightType::POINT);

  LOG_TRACE("PointLight GPU data prepared - position: ({}, {}, {}), radius: {}",
            gpuData.position.x,
            gpuData.position.y,
            gpuData.position.z,
            m_Properties.specific.point.radius);

  return gpuData;
}

float PointLight::CalculateInfluenceRadius() const
{
  if (!m_Properties.enabled) {
    return 0.0f;
  }

  // 基于光源强度和衰减计算粗略的影响半径（简化计算）
  float baseRadius = m_Properties.specific.point.radius;
  float intensityFactor = glm::sqrt(m_Properties.intensity);

  // 影响半径 = 基础半径 × 强度因子
  float influenceRadius = baseRadius * intensityFactor;

  LOG_TRACE("PointLight influence radius calculated: {} (base: {}, intensity: {})",
            influenceRadius,
            baseRadius,
            m_Properties.intensity);

  return influenceRadius;
}

void PointLight::CreateDefaultShadowMap()
{
  // 创建点光源阴影贴图数据
  ShadowMapData shadowData;
  shadowData.enabled = true;
  shadowData.quality = ShadowQuality::MEDIUM;
  shadowData.filter = ShadowFilter::PCF;
  shadowData.bias = 0.005f;
  shadowData.normalBias = 0.01f;

  // 设置点光源特定的阴影参数
  shadowData.specific.point.nearPlane = 0.1f;
  shadowData.specific.point.farPlane = m_Properties.specific.point.radius;

  // 创建点光源阴影贴图实例
  m_ShadowMap = std::make_shared<PointShadowMap>(shadowData);

  LOG_TRACE("Default PointShadowMap created for PointLight - farPlane: {}",
            shadowData.specific.point.farPlane);
}

void PointLight::SetRadius(float radius)
{
  if (radius <= 0.0f) {
    LOG_ERROR("PointLight radius must be positive: {}", radius);
    return;
  }

  m_Properties.specific.point.radius = radius;
  LOG_TRACE("PointLight radius set to: {}", radius);
}

float PointLight::GetRadius() const
{
  return m_Properties.specific.point.radius;
}

void PointLight::SetFalloff(float falloff)
{
  if (falloff <= 0.0f) {
    LOG_ERROR("PointLight falloff must be positive: {}", falloff);
    return;
  }

  m_Properties.specific.point.falloff = falloff;
  LOG_TRACE("PointLight falloff set to: {}", falloff);
}

float PointLight::GetFalloff() const
{
  return m_Properties.specific.point.falloff;
}

void PointLight::SetAttenuation(LightAttenuation attenuation)
{
  m_Properties.specific.point.attenuation = attenuation;
  LOG_TRACE("PointLight attenuation mode set to: {}", static_cast<int>(attenuation));
}

LightAttenuation PointLight::GetAttenuation() const
{
  return m_Properties.specific.point.attenuation;
}

bool PointLight::Validate() const
{
  // 首先验证基础参数
  if (!ValidateBaseParameters()) {
    return false;
  }

  // 然后验证点光源特定参数
  return ValidatePointLightParameters();
}

bool PointLight::ValidatePointLightParameters() const
{
  if (m_Properties.specific.point.radius <= 0.0f) {
    LOG_ERROR("PointLight radius must be positive: {}", m_Properties.specific.point.radius);
    return false;
  }

  if (m_Properties.specific.point.falloff <= 0.0f) {
    LOG_ERROR("PointLight falloff must be positive: {}", m_Properties.specific.point.falloff);
    return false;
  }

  // 检查强度是否合理（可选，根据项目需求调整）
  if (m_Properties.intensity > 100000.0f) {
    LOG_WARN("PointLight intensity is very high: {}", m_Properties.intensity);
  }

  return true;
}

}  // namespace mite
