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
   m_Lights.clear();

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

std::shared_ptr<Light> LightManager::CreatePointLight()
{
  auto light = std::make_shared<PointLight>();
  if (AddLight(light)) {
    LOG_TRACE("Point light created and added to manager");
    return light;
  }
  return nullptr;
}
// std::shared_ptr<Light> LightManager::CreateSpotLight()
//{
//   auto light = std::make_shared<SpotLight>();
//   if (AddLight(light)) {
//     LOG_TRACE("Spot light created and added to manager");
//     return light;
//   }
//   return nullptr;
// }
// std::shared_ptr<Light> LightManager::CreateDirectionalLight()
//{
//   auto light = std::make_shared<DirectionalLight>();
//   if (AddLight(light)) {
//     LOG_TRACE("Directional light created and added to manager");
//     return light;
//   }
//   return nullptr;
// }
// std::shared_ptr<Light> LightManager::CreateAreaRectLight()
//{
//   auto light = std::make_shared<AreaRectLight>();
//   if (AddLight(light)) {
//     LOG_TRACE("Area rectangle light created and added to manager");
//     return light;
//   }
//   return nullptr;
// }
// std::shared_ptr<Light> LightManager::CreateAreaEllipseLight()
//{
//   auto light = std::make_shared<AreaEllipseLight>();
//   if (AddLight(light)) {
//     LOG_TRACE("Area ellipse light created and added to manager");
//     return light;
//   }
//   return nullptr;
// }

std::shared_ptr<Light> LightManager::CreateLight(LightType type)
{
  switch (type) {
    case LightType::POINT:
      return CreatePointLight();
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

bool LightManager::AddLight(std::shared_ptr<Light> light)
{
  if (!m_IsInitialized) {
    LOG_ERROR("Cannot add light: LightManager not initialized");
    return false;
  }

  if (!light) {
    LOG_ERROR("Cannot add null light");
    return false;
  }

  if (!CanAddLight(light)) {
    LOG_ERROR("Cannot add light: exceeds max limit or already exists");
    return false;
  }

  // 验证光源参数
  if (!light->Validate()) {
    LOG_ERROR("Cannot add invalid light");
    return false;
  }

  // 添加到光源列表
  m_Lights.push_back(light);

  LOG_TRACE("Light added: type={}, total lights={}", light->GetLightTypeName(), m_Lights.size());

  return true;
}

bool LightManager::RemoveLight(std::shared_ptr<Light> light)
{
  if (!light) {
    LOG_ERROR("Cannot remove null light");
    return false;
  }

  auto it = std::find(m_Lights.begin(), m_Lights.end(), light);
  if (it == m_Lights.end()) {
    LOG_WARN("Light not found in manager, cannot remove");
    return false;
  }

  m_Lights.erase(it);

  LOG_TRACE(
      "Light removed: type={}, remaining lights={}", light->GetLightTypeName(), m_Lights.size());

  return true;
}

void LightManager::ClearAllLights()
{
  size_t previousCount = m_Lights.size();
  m_Lights.clear();

  LOG_INFO("Cleared all {} lights from LightManager", previousCount);
}

const std::vector<std::shared_ptr<Light>> &LightManager::GetAllLights() const
{
  return m_Lights;
}

std::vector<std::shared_ptr<Light>> LightManager::GetEnabledLights() const
{
  std::vector<std::shared_ptr<Light>> enabledLights;
  std::copy_if(m_Lights.begin(),
               m_Lights.end(),
               std::back_inserter(enabledLights),
               [](const std::shared_ptr<Light> &light) { return light->IsEnabled(); });
  return enabledLights;
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

//std::vector<ShadowMapData> LightManager::CollectShadowData(
//    const std::unordered_map<std::shared_ptr<Light>, Transform> &worldTransforms,
//    const Transform &cameraView,
//    const glm::mat4 &cameraProj) const
//{
//  std::vector<ShadowMapData> shadowDataList;
//
//  for (const auto &[light, transform] : worldTransforms) {
//    ShadowMapData shadowData = light->PrepareShadowData(transform, cameraView, cameraProj);
//    if (shadowData.isValid) {
//      shadowDataList.push_back(shadowData);
//    }
//  }
//
//  LOG_TRACE("Collected shadow data for {} lights", shadowDataList.size());
//  return shadowDataList;
//}

size_t LightManager::GetLightCountByType(
    LightType type) const
{
  size_t lightCount = 0;

  for (const auto &light : m_Lights) {
    if (light && light->GetType() == type) {
      lightCount++;
    }
  }

  return lightCount;
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

bool LightManager::CanAddLight(std::shared_ptr<Light> light) const
{
  // 检查是否超过最大限制
  if (m_Lights.size() >= m_MaxLights) {
    LOG_ERROR("Cannot add light: reached max limit of {} lights", m_MaxLights);
    return false;
  }

  // 检查是否已存在
  auto it = std::find(m_Lights.begin(), m_Lights.end(), light);
  if (it != m_Lights.end()) {
    LOG_WARN("Light already exists in manager");
    return false;
  }

  return true;
}

}  // namespace mite