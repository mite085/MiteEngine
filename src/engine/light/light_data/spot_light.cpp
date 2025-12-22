#include "spot_light.h"

namespace mite {
SpotLight::SpotLight() : Light(LightType::SPOT) {
  // 聚光灯的默认参数已在Light基类构造函数中设置
  LOG_TRACE("SpotLight created with innerAngle: {}, outerAngle: {}, range: {}",
            m_Properties.specific.spot.innerAngle,
            m_Properties.specific.spot.outerAngle,
            m_Properties.specific.spot.range);

  CreateDefaultShadowMap();
}

std::string SpotLight::GetLightTypeName() const { return "SpotLight"; }

GPULightData SpotLight::PrepareGPULightData(const Transform &worldTransform,
                                            int typeLocalIndex) const {
  if (!m_Properties.enabled) {
    LOG_WARN("Preparing GPU data for disabled spot light");
  }

  // 使用基类的GPULightData构造函数，传入聚光灯类型
  GPULightData gpuData(m_Properties, worldTransform, LightType::SPOT,
                       typeLocalIndex);

  LOG_TRACE(
      "SpotLight GPU data prepared - position: ({}, {}, {}), direction: ({}, "
      "{}, {}), range: {}",
      gpuData.position.x, gpuData.position.y, gpuData.position.z,
      gpuData.direction.x, gpuData.direction.y, gpuData.direction.z,
      m_Properties.specific.spot.range);

  return gpuData;
}

float SpotLight::CalculateInfluenceRadius() const {
  if (!m_Properties.enabled) {
    return 0.0f;
  }

  // 聚光灯的影响半径就是其照射范围
  float influenceRadius = m_Properties.specific.spot.range;

  LOG_TRACE("SpotLight influence radius calculated: {}", influenceRadius);

  return influenceRadius;
}

void SpotLight::CreateDefaultShadowMap() {
  // 创建聚光灯阴影贴图数据
  ShadowMapData shadowData;
  shadowData.enabled = true;

  // 设置聚光灯特定的阴影参数
  shadowData.specific.spot.nearPlane = 0.1f;
  shadowData.specific.spot.farPlane = m_Properties.specific.spot.range;

  // 创建聚光灯阴影贴图实例
  m_ShadowMap = std::make_shared<SpotShadowMap>(shadowData);

  LOG_TRACE("Default SpotShadowMap created for SpotLight - farPlane: {}",
            shadowData.specific.spot.farPlane);
}

void SpotLight::SetInnerAngle(float angle) {
  if (angle <= 0.0f || angle >= 180.0f) {
    LOG_ERROR("SpotLight inner angle must be between 0 and 180 degrees: {}",
              angle);
    return;
  }

  if (angle > m_Properties.specific.spot.outerAngle) {
    LOG_ERROR(
        "SpotLight inner angle ({}) cannot be larger than outer angle ({})",
        angle, m_Properties.specific.spot.outerAngle);
    return;
  }

  m_Properties.specific.spot.innerAngle = angle;
  LOG_TRACE("SpotLight inner angle set to: {}", angle);
}

float SpotLight::GetInnerAngle() const {
  return m_Properties.specific.spot.innerAngle;
}

void SpotLight::SetOuterAngle(float angle) {
  if (angle <= 0.0f || angle >= 180.0f) {
    LOG_ERROR("SpotLight outer angle must be between 0 and 180 degrees: {}",
              angle);
    return;
  }

  if (angle < m_Properties.specific.spot.innerAngle) {
    LOG_ERROR(
        "SpotLight outer angle ({}) cannot be smaller than inner angle ({})",
        angle, m_Properties.specific.spot.innerAngle);
    return;
  }

  m_Properties.specific.spot.outerAngle = angle;
  LOG_TRACE("SpotLight outer angle set to: {}", angle);
}

float SpotLight::GetOuterAngle() const {
  return m_Properties.specific.spot.outerAngle;
}

void SpotLight::SetBlend(float blend) {
  if (blend < 0.0f || blend > 1.0f) {
    LOG_ERROR("SpotLight blend must be between 0 and 1: {}", blend);
    return;
  }

  m_Properties.specific.spot.blend = blend;
  LOG_TRACE("SpotLight blend set to: {}", blend);
}

float SpotLight::GetBlend() const { return m_Properties.specific.spot.blend; }

void SpotLight::SetRange(float range) {
  if (range <= 0.0f) {
    LOG_ERROR("SpotLight range must be positive: {}", range);
    return;
  }

  m_Properties.specific.spot.range = range;
  LOG_TRACE("SpotLight range set to: {}", range);
}

float SpotLight::GetRange() const { return m_Properties.specific.spot.range; }

bool SpotLight::Validate() const {
  // 首先验证基础参数
  if (!ValidateBaseParameters()) {
    return false;
  }

  // 然后验证聚光灯特定参数
  return ValidateSpotLightParameters();
}

bool SpotLight::ValidateSpotLightParameters() const {
  if (m_Properties.specific.spot.innerAngle <= 0.0f ||
      m_Properties.specific.spot.innerAngle >= 180.0f) {
    LOG_ERROR("SpotLight inner angle must be between 0 and 180 degrees: {}",
              m_Properties.specific.spot.innerAngle);
    return false;
  }

  if (m_Properties.specific.spot.outerAngle <= 0.0f ||
      m_Properties.specific.spot.outerAngle >= 180.0f) {
    LOG_ERROR("SpotLight outer angle must be between 0 and 180 degrees: {}",
              m_Properties.specific.spot.outerAngle);
    return false;
  }

  if (m_Properties.specific.spot.innerAngle >
      m_Properties.specific.spot.outerAngle) {
    LOG_ERROR(
        "SpotLight inner angle ({}) cannot be larger than outer angle ({})",
        m_Properties.specific.spot.innerAngle,
        m_Properties.specific.spot.outerAngle);
    return false;
  }

  if (m_Properties.specific.spot.blend < 0.0f ||
      m_Properties.specific.spot.blend > 1.0f) {
    LOG_ERROR("SpotLight blend must be between 0 and 1: {}",
              m_Properties.specific.spot.blend);
    return false;
  }

  if (m_Properties.specific.spot.range <= 0.0f) {
    LOG_ERROR("SpotLight range must be positive: {}",
              m_Properties.specific.spot.range);
    return false;
  }

  // 检查强度是否合理（仅报warn，不返回错误信息）
  if (m_Properties.intensity > 100000.0f) {
    LOG_WARN("SpotLight intensity is very high: {}", m_Properties.intensity);
  }

  return true;
}
}  // namespace mite