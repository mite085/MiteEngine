#include "directional_light.h"

namespace mite {
DirectionalLight::DirectionalLight() : Light(LightType::DIRECTIONAL)
{
  // 点光源的默认参数已在Light基类构造函数中设置
  LOG_TRACE("DirectionalLight created with irradiance: {}, intensity: {}",
            m_Properties.specific.directional.irradiance,
            m_Properties.intensity);

  CreateDefaultShadowMap();
}

std::string DirectionalLight::GetLightTypeName() const
{
  return "DirectionalLight";
}

GPULightData DirectionalLight::PrepareGPULightData(const Transform &worldTransform,
                                                   int typeLocalIndex) const
{
  if (!m_Properties.enabled) {
    LOG_WARN("Preparing GPU data for disabled direction light");
  }

  // 使用基类的GPULightData构造函数，传入方向光类型
  GPULightData gpuData(m_Properties, worldTransform, LightType::DIRECTIONAL, typeLocalIndex);

  LOG_TRACE("DirectionalLight GPU data prepared - position: ({}, {}, {}), irradiance: {}",
            gpuData.position.x,
            gpuData.position.y,
            gpuData.position.z,
            m_Properties.specific.directional.irradiance);

  return gpuData;
}

float DirectionalLight::CalculateInfluenceRadius() const
{
  if (!m_Properties.enabled) {
    return 0.0f;
  }

  // 方向光无差别照射，影响半径应当是无穷大
  return FLT_MAX;
}

void DirectionalLight::CreateDefaultShadowMap()
{
  // 创建方向光阴影贴图数据
  ShadowMapData shadowData;
  shadowData.enabled = true;

  // 设置方向光特定的阴影参数
  shadowData.specific.directional.cascadeCount = 4;
  shadowData.specific.directional.splitLambda = 0.95f;

  // 创建方向光阴影贴图实例
  m_ShadowMap = std::make_shared<DirectionalShadowMap>(shadowData);

  LOG_TRACE("Default Directional ShadowMap created for Directional Light - farPlane: {}");
}

bool DirectionalLight::Validate() const
{
  // 首先验证基础参数
  if (!ValidateBaseParameters()) {
    return false;
  }

  // 然后验证方向光特定参数
  return ValidateDirectionalLightParameters();
}

void DirectionalLight::SetIrradius(float radius)
{
  m_Properties.specific.directional.irradiance = radius;
}

float DirectionalLight::GetIrradius() const
{
  return m_Properties.specific.directional.irradiance;
}

bool DirectionalLight::ValidateDirectionalLightParameters() const
{

  // 检查强度是否合理（仅报warn，不返回错误信息）
  if (m_Properties.intensity > 100000.0f) {
    LOG_WARN("DirectionalLight intensity is very high: {}", m_Properties.intensity);
  }

  return true;
}
}  // namespace mite