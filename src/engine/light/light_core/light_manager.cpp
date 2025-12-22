#include "light_manager.h"

#include "basic_event/instance_event.h"
#include "light_data/directional_light.h"
#include "light_data/point_light.h"
#include "light_data/spot_light.h"
#include "subscription_group.h"

namespace mite {
LightManager::LightManager(size_t maxLights) : m_MaxLights(maxLights) {
  LOG_TRACE("LightManager created with max lights: {}", maxLights);
}

bool LightManager::Initialize() {
  if (m_IsInitialized) {
    LOG_WARN("LightManager already initialized");
    return true;
  }

  try {
    // 创建LightSSBO实例
    m_LightSSBO = std::make_shared<LightShaderStorgeBuffer>(m_MaxLights);
    m_LightSSBO->Initialize();

    EventBus::Publish<LightSSBOCreateEvent>(m_LightSSBO);

    LOG_INFO("LightManager initialized successfully with {} max lights",
             m_MaxLights);

    // 初始化shadowmap UBO
    InitializeShadowInstance();
    m_IsInitialized = true;
    return true;
  } catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize LightManager: {}", e.what());
    return false;
  }
}

void LightManager::Destroy() {
  if (!m_IsInitialized) {
    return;
  }

  // 清理光源列表
  m_Lights.clear();
  m_LightTransformCache.clear();

  // 销毁SSBO
  if (m_LightSSBO) {
    m_LightSSBO->Destroy();
    m_LightSSBO.reset();
  }

  m_IsInitialized = false;
  LOG_INFO("LightManager destroyed");
}

bool LightManager::IsInitialized() const { return m_IsInitialized; }

std::shared_ptr<Light> LightManager::CreatePointLight() {
  auto light = std::make_shared<PointLight>();
  if (AddLight(light)) {
    LOG_TRACE("Point light created and added to manager");
    return light;
  }
  return nullptr;
}
std::shared_ptr<Light> LightManager::CreateSpotLight() {
  auto light = std::make_shared<SpotLight>();
  if (AddLight(light)) {
    LOG_TRACE("Spot light created and added to manager");
    return light;
  }
  return nullptr;
}
std::shared_ptr<Light> LightManager::CreateDirectionalLight() {
  auto light = std::make_shared<DirectionalLight>();
  if (AddLight(light)) {
    LOG_TRACE("Directional light created and added to manager");
    return light;
  }
  return nullptr;
}
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

std::shared_ptr<Light> LightManager::CreateLight(LightType type) {
  switch (type) {
    case LightType::POINT:
      return CreatePointLight();
    case LightType::SPOT:
      return CreateSpotLight();
    case LightType::DIRECTIONAL:
      return CreateDirectionalLight();
    // case LightType::AREA_RECT:
    //   return CreateAreaRectLight();
    // case LightType::AREA_ELLIPSE:
    //   return CreateAreaEllipseLight();
    default:
      LOG_ERROR("Unknown light type: {}", static_cast<int>(type));
      return nullptr;
  }
}

bool LightManager::AddLight(std::shared_ptr<Light> light) {
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

  LOG_TRACE("Light added: type={}, total lights={}", light->GetLightTypeName(),
            m_Lights.size());

  return true;
}

bool LightManager::RemoveLight(std::shared_ptr<Light> light) {
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

  LOG_TRACE("Light removed: type={}, remaining lights={}",
            light->GetLightTypeName(), m_Lights.size());

  return true;
}

void LightManager::ClearAllLights() {
  size_t previousCount = m_Lights.size();
  m_Lights.clear();

  LOG_INFO("Cleared all {} lights from LightManager", previousCount);
}

const std::vector<std::shared_ptr<Light>> &LightManager::GetAllLights() const {
  return m_Lights;
}

std::vector<std::shared_ptr<Light>> LightManager::GetEnabledLights() const {
  std::vector<std::shared_ptr<Light>> enabledLights;
  std::copy_if(
      m_Lights.begin(), m_Lights.end(), std::back_inserter(enabledLights),
      [](const std::shared_ptr<Light> &light) { return light->IsEnabled(); });
  return enabledLights;
}

bool LightManager::UpdateLightData(
    const std::unordered_map<Light *, Transform> &worldTransforms) {
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
  } else {
    LOG_ERROR("Failed to update lights to GPU");
  }

  return success;
}

// std::vector<ShadowMapData> LightManager::CollectShadowData(
//     const std::unordered_map<std::shared_ptr<Light>, Transform>
//     &worldTransforms, const Transform &cameraView, const glm::mat4
//     &cameraProj) const
//{
//   std::vector<ShadowMapData> shadowDataList;
//
//   for (const auto &[light, transform] : worldTransforms) {
//     ShadowMapData shadowData = light->PrepareShadowData(transform,
//     cameraView, cameraProj); if (shadowData.isValid) {
//       shadowDataList.push_back(shadowData);
//     }
//   }
//
//   LOG_TRACE("Collected shadow data for {} lights", shadowDataList.size());
//   return shadowDataList;
// }

size_t LightManager::GetLightCountByType(LightType type) const {
  size_t lightCount = 0;

  // 遍历执行计数操作（光源数量上百已是极限，无需考虑此处的时间复杂度优化）
  for (const auto &light : m_Lights) {
    if (light && light->GetType() == type) {
      lightCount++;
    }
  }

  return lightCount;
}
std::vector<std::shared_ptr<Light>> LightManager::GetLightsByType(
    LightType type) const {
  std::vector<std::shared_ptr<Light>> lights;

  // 遍历按照类型获取（光源数量上百已是极限，无需考虑此处的时间复杂度优化）
  for (const auto &light : m_Lights) {
    if (light && light->GetType() == type) {
      lights.push_back(light);
    }
  }

  return lights;
}
std::shared_ptr<LightShaderStorgeBuffer> LightManager::GetLightSSBO() const {
  return m_LightSSBO;
}

bool LightManager::InitializeShadowInstance() {
  if (m_ShadowInstance && m_ShadowInstance->GetUBO() &&
      m_ShadowInstance->GetUBO()->IsInitialized()) {
    LOG_WARN("ShadowInstance already initialized");
    return true;
  }
  try {
    // 创建阴影实例
    m_ShadowInstance = std::make_shared<ShadowInstance>();
    bool success = m_ShadowInstance->InitializeUBO();

    if (success) {
      LOG_INFO("ShadowInstance initialized successfully");
    } else {
      LOG_ERROR("Failed to initialize ShadowInstance UBO");
      m_ShadowInstance.reset();
    }

    return success;
  } catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize ShadowInstance: {}", e.what());
    m_ShadowInstance.reset();
    return false;
  }
}

bool LightManager::UpdateLightShadowUBO(
    std::shared_ptr<CameraInstance> cameraInstance, glm::vec4 shadowParams) {
  if (!m_ShadowInstance || !m_ShadowInstance->GetUBO() ||
      !m_ShadowInstance->GetUBO()->IsInitialized()) {
    LOG_ERROR("Cannot update light shadow: ShadowInstance not initialized");
    return false;
  }
  if (!cameraInstance) {
    LOG_ERROR("Cannot update light shadow: null camera instance");
    return false;
  }
  if (m_LightTransformCache.empty()) {
    LOG_WARN("No light transform cache available, call UpdateLightData first");
    return false;
  }
  try {
    // 获取所有启用的光源
    auto enabledLights = GetEnabledLights();

    LOG_TRACE("Updating shadow data for {} enabled lights",
              enabledLights.size());
    // 传递光源列表和变换缓存给ShadowInstance
    return m_ShadowInstance->UpdateUBO(enabledLights, m_LightTransformCache,
                                       cameraInstance, shadowParams);
  } catch (const std::exception &e) {
    LOG_ERROR("Exception while updating light shadow: {}", e.what());
    return false;
  }
}

void LightManager::SetMaxLights(size_t maxLights) {
  if (m_IsInitialized) {
    LOG_WARN("Cannot change max lights after initialization");
    return;
  }

  m_MaxLights = maxLights;
  LOG_TRACE("LightManager max lights set to: {}", maxLights);
}

size_t LightManager::GetMaxLights() const { return m_MaxLights; }

Transform LightManager::GetLightTransform(Light *lightPtr) const {
  // 检查light ptr合法性
  if (!lightPtr) {
    LOG_ERROR("LightManager invalid light ptr.");
    return Transform();
  }

  // 获取light变换的缓存
  if (m_LightTransformCache.find(lightPtr) == m_LightTransformCache.end()) {
    LOG_ERROR("LightManager invalid light transform cache.");
    return Transform();
  } else {
    return m_LightTransformCache.at(lightPtr);
  }
}

// ---- 内部方法实现 ----

std::vector<GPULightData> LightManager::PrepareGPULightData(
    const std::unordered_map<Light *, Transform> &worldTransforms) const {
  std::vector<GPULightData> gpuData;
  gpuData.reserve(worldTransforms.size());

  // 清空变换缓存
  m_LightTransformCache.clear();

  // 为每种光源类型维护计数器
  std::unordered_map<LightType, int> typeCounters = {
      {LightType::POINT, 0},
      {LightType::SPOT, 0},
      {LightType::DIRECTIONAL, 0},
      {LightType::AREA_RECT, 0},
      {LightType::AREA_ELLIPSE, 0}};

  // 第一遍：计算每个光源的类型内索引
  std::unordered_map<Light *, int> lightTypeIndices;

  for (std::shared_ptr<Light> light : m_Lights) {
    if (!light || !light->IsEnabled()) {
      continue;
    }
    LightType lightType = light->GetType();
    int typeLocalIndex = typeCounters[lightType];
    lightTypeIndices[light.get()] = typeLocalIndex;
    typeCounters[lightType]++;
  }

  // 第二遍：准备GPU数据
  for (std::shared_ptr<Light> light : m_Lights) {
    if (!light || !light->IsEnabled()) {
      continue;
    }
    try {
      // 获取光源的世界变换
      auto it = worldTransforms.find(light.get());
      if (it == worldTransforms.end()) {
        LOG_WARN("Light transform not found for light: {}",
                 light->GetLightTypeName());
        continue;
      }
      Transform transform = it->second;

      // 获取类型内索引
      int typeLocalIndex = lightTypeIndices[light.get()];

      // 调用光源的PrepareGPULightData函数，传递类型内索引
      GPULightData lightData =
          light->PrepareGPULightData(transform, typeLocalIndex);

      gpuData.push_back(lightData);
      m_LightTransformCache.insert({light.get(), transform});

      LOG_TRACE("Prepared GPU data for {} light, typeLocalIndex: {}",
                light->GetLightTypeName(), typeLocalIndex);
    } catch (const std::exception &e) {
      LOG_ERROR("Failed to prepare GPU data for light: {}", e.what());
    }
  }
  // 记录统计信息
  LOG_TRACE(
      "Light type statistics - Point: {}, Spot: {}, Directional: {}, AreaRect: "
      "{}, AreaEllipse: "
      "{}",
      typeCounters[LightType::POINT], typeCounters[LightType::SPOT],
      typeCounters[LightType::DIRECTIONAL], typeCounters[LightType::AREA_RECT],
      typeCounters[LightType::AREA_ELLIPSE]);
  return gpuData;
}

bool LightManager::CanAddLight(std::shared_ptr<Light> light) const {
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