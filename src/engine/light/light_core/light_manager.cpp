#include "light_manager.h"
#include "basic_event/instance_event.h"
#include "light_data/point_light.h"
#include "subscription_group.h"

namespace mite {
LightManager::LightManager(size_t maxLights) : m_MaxLights(maxLights)
{
  LOG_TRACE("LightManager created with max lights: {}", maxLights);
}

bool LightManager::Initialize()
{
  if (m_IsInitialized) {
    LOG_WARN("LightManager already initialized");
    return true;
  }

  try {
    // 创建LightSSBO实例
    m_LightSSBO = std::make_shared<LightShaderStorgeBuffer>(m_MaxLights);
    m_LightSSBO->Initialize();

    EventBus::Publish<LightSSBOCreateEvent>(LightSSBOCreateEvent(m_LightSSBO));

    LOG_INFO("LightManager initialized successfully with {} max lights", m_MaxLights);
    m_IsInitialized = true;
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize LightManager: {}", e.what());
    return false;
  }
}

void LightManager::Destroy()
{
  if (!m_IsInitialized) {
    return;
  }

  // 清理光源列表
  // m_Lights.clear();

  // 销毁SSBO
  if (m_LightSSBO) {
    m_LightSSBO->Destroy();
    m_LightSSBO.reset();
  }

  m_IsInitialized = false;
  LOG_INFO("LightManager destroyed");
}

bool LightManager::IsInitialized() const
{
  return m_IsInitialized;
}

std::shared_ptr<Light> LightManager::CreateLight(LightType type)
{
  switch (type) {
    case LightType::POINT:
      return std::make_shared<PointLight>();
    // case LightType::SPOT:
    //   return CreateSpotLight();
    // case LightType::DIRECTIONAL:
    //   return CreateDirectionalLight();
    // case LightType::AREA_RECT:
    //   return CreateAreaRectLight();
    // case LightType::AREA_ELLIPSE:
    //   return CreateAreaEllipseLight();
    default:
      LOG_ERROR("Unknown light type: {}", static_cast<int>(type));
      return nullptr;
  }
}

bool LightManager::UpdateLightData(
    const std::unordered_map<std::shared_ptr<Light>, Transform> &worldTransforms)
{
  if (!m_IsInitialized || !m_LightSSBO) {
    LOG_ERROR("Cannot update light data: LightManager not initialized");
    return false;
  }

  // 准备GPU光源数据
  auto gpuLightData = PrepareGPULightData(worldTransforms);

  // 更新到SSBO
  bool success = m_LightSSBO->UpdateLights(gpuLightData);

  if (success) {
    LOG_TRACE("Updated {} lights to GPU", gpuLightData.size());
  }
  else {
    LOG_ERROR("Failed to update lights to GPU");
  }

  return success;
}

std::vector<ShadowMapData> LightManager::CollectShadowData(
    const std::unordered_map<std::shared_ptr<Light>, Transform> &worldTransforms,
    const Transform &cameraView,
    const glm::mat4 &cameraProj) const
{
  std::vector<ShadowMapData> shadowDataList;

  for (const auto &[light, transform] : worldTransforms) {
    ShadowMapData shadowData = light->PrepareShadowData(transform, cameraView, cameraProj);
    if (shadowData.isValid) {
      shadowDataList.push_back(shadowData);
    }
  }

  LOG_TRACE("Collected shadow data for {} lights", shadowDataList.size());
  return shadowDataList;
}

std::shared_ptr<LightShaderStorgeBuffer> LightManager::GetLightSSBO() const
{
  return m_LightSSBO;
}

void LightManager::SetMaxLights(size_t maxLights)
{
  if (m_IsInitialized) {
    LOG_WARN("Cannot change max lights after initialization");
    return;
  }

  m_MaxLights = maxLights;
  LOG_TRACE("LightManager max lights set to: {}", maxLights);
}

size_t LightManager::GetMaxLights() const
{
  return m_MaxLights;
}

// ---- 内部方法实现 ----

std::vector<GPULightData> LightManager::PrepareGPULightData(
    const std::unordered_map<std::shared_ptr<Light>, Transform> &worldTransforms) const
{
  std::vector<GPULightData> gpuData;

  gpuData.reserve(worldTransforms.size());

  for (const auto &[light, transform] : worldTransforms) {
    try {
      GPULightData lightData = light->PrepareGPULightData(transform);
      gpuData.push_back(lightData);
    }
    catch (const std::exception &e) {
      LOG_ERROR("Failed to prepare GPU data for light: {}", e.what());
    }
  }

  return gpuData;
}
}  // namespace mite