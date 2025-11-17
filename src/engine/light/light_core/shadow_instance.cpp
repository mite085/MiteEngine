#include "shadow_instance.h"
#include "basic_shader/shader_binding_point_manager.h"
#include "light_data/directional_light.h"
#include "light_data/point_light.h"
#include "light_data/spot_light.h"

namespace mite {
ShadowInstance::ShadowInstance()
{
  LOG_DEBUG("ShadowInstance allocated");
}

ShadowInstance::~ShadowInstance()
{
  if (m_ShadowUBO && m_ShadowUBO->IsInitialized()) {
    m_ShadowUBO->Destroy();
  }
}

bool ShadowInstance::InitializeUBO()
{
  if (m_ShadowUBO && m_ShadowUBO->IsInitialized()) {
    LOG_WARN("ShadowInstance UBO already initialized");
    return true;
  }

  try {
    // 创建阴影UBO对象
    m_ShadowUBO = std::make_shared<ShaderUBO>(sizeof(ShadowUniformBuffer),
                                              BindingPointManager::Get().GetShadowUBOBinding(),
                                              GL_DYNAMIC_DRAW);
    m_ShadowUBO->Initialize();

    // 初始化阴影数据为默认值
    m_ShadowData = ShadowUniformBuffer{};

    LOG_DEBUG("ShadowInstance UBO initialized successfully");
    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to initialize ShadowInstance UBO: {}", e.what());
    m_ShadowUBO.reset();
    return false;
  }
}

bool ShadowInstance::UpdateUBO(const std::vector<std::shared_ptr<Light>> &lights,
                               const std::unordered_map<Light *, Transform> &lightTransforms,
                               std::shared_ptr<CameraInstance> cameraInstance)
{
  if (!m_ShadowUBO || !m_ShadowUBO->IsInitialized()) {
    LOG_ERROR("Cannot update UBO: ShadowInstance UBO not initialized");
    return false;
  }
  if (!cameraInstance) {
    LOG_ERROR("Cannot update UBO: null camera instance");
    return false;
  }
  if (lightTransforms.empty()) {
    LOG_WARN("No light transforms provided");
    return false;
  }
  try {
    // 重置阴影配置
    m_ShadowData.shadowConfig = glm::ivec4(0);

    // 重置计数器
    uint32_t directionalCount = 0;
    uint32_t pointCount = 0;
    uint32_t spotCount = 0;
    // 遍历所有光源，按类型处理
    for (const auto &light : lights) {
      if (!light || !light->IsEnabled()) {
        continue;
      }
      // 查找光源变换
      auto transformIt = lightTransforms.find(light.get());
      if (transformIt == lightTransforms.end()) {
        LOG_WARN("Light transform not found for light");
        continue;
      }
      // 获取光源类型
      LightType type = light->GetType();

      switch (type) {
        case LightType::DIRECTIONAL: {
          if (directionalCount >= MAX_DIRECTIONAL_LIGHTS) {
            LOG_WARN("Exceeded maximum directional lights for shadows: {}",
                     MAX_DIRECTIONAL_LIGHTS);
            continue;
          }

          if (ProcessDirectionalLight(
                  light, transformIt->second, directionalCount, cameraInstance))
          {
            directionalCount++;
          }
          break;
        }

        case LightType::POINT: {
          if (pointCount >= MAX_POINT_LIGHTS) {
            LOG_WARN("Exceeded maximum point lights for shadows: {}", MAX_POINT_LIGHTS);
            continue;
          }

          if (ProcessPointLight(light, transformIt->second, pointCount, cameraInstance)) {
            pointCount++;
          }
          break;
        }

        case LightType::SPOT: {
          if (spotCount >= MAX_SPOT_LIGHTS) {
            LOG_WARN("Exceeded maximum spot lights for shadows: {}", MAX_SPOT_LIGHTS);
            continue;
          }

          if (ProcessSpotLight(light, transformIt->second, spotCount, cameraInstance)) {
            spotCount++;
          }
          break;
        }

        default:
          // 其他类型的光源不支持阴影，静默跳过
          LOG_TRACE("Light type {} does not support shadows", static_cast<int>(type));
          break;
      }
    }

    // 更新阴影配置
    m_ShadowData.shadowConfig = glm::ivec4(directionalCount, pointCount, spotCount, 0);

    // 设置级联数量（TODO: 使用正确的级联数量，此处默认使用最大值）
    m_ShadowData.shadowConfig.w = MAX_CASCADES;

    // 设置通用阴影参数
    // TODO:
    // ShadowMap分辨率在这里设置并不合适，应当作为输入参数传入。
    // 但着色器似乎也无需该参数执行计算）
    m_ShadowData.shadowParams = glm::vec4(0.005f, 0.02f, 1.0f, 1024.0f);
    // 更新UBO数据
    bool success = m_ShadowUBO->UpdateData(&m_ShadowData, sizeof(ShadowUniformBuffer));
    if (success) {
      LOG_TRACE("ShadowInstance UBO updated - Directional: {}, Point: {}, Spot: {}",
                directionalCount,
                pointCount,
                spotCount);
    }
    else {
      LOG_ERROR("Failed to update ShadowInstance UBO data");
    }
    return success;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Exception while updating ShadowInstance UBO: {}", e.what());
    return false;
  }
}

void ShadowInstance::BindUBO()
{
  if (!m_ShadowUBO || !m_ShadowUBO->IsInitialized()) {
    LOG_ERROR("Cannot bind UBO: ShadowInstance UBO not initialized");
    return;
  }

  // 绑定阴影UBO
  m_ShadowUBO->Bind();

  // LOG_TRACE("ShadowInstance UBO bound");
}

bool ShadowInstance::ProcessDirectionalLight(std::shared_ptr<Light> light,
                                             const Transform &lightTransform,
                                             uint32_t lightIndex,
                                             std::shared_ptr<CameraInstance> cameraInstance)
{
  // 类型转换
  auto directionalLight = std::dynamic_pointer_cast<DirectionalLight>(light);
  if (!directionalLight) {
    LOG_ERROR("Failed to cast to DirectionalLight");
    return false;
  }

  // 准备阴影数据
  ShadowMapData shadowData = light->PrepareShadowData(lightIndex,
                                                      lightTransform,
                                                      cameraInstance->GetCameraTransform(),
                                                      cameraInstance->GetProjectionMatrix());
  if (!shadowData.enabled || !shadowData.isValid) {
    return false;
  }
  // 设置阴影索引
  m_ShadowData.directionalShadowIndices[lightIndex] = glm::ivec4(
      shadowData.shadowMapIndex, 0, 0, 0);

  // 设置级联分割距离
  for (uint32_t cascadeIdx = 0;
       cascadeIdx < shadowData.specific.directional.cascadeCount && cascadeIdx < MAX_CASCADES;
       cascadeIdx++)
  {
    m_ShadowData.cascadeSplits[cascadeIdx].x =
        shadowData.specific.directional.cascadeSplits[cascadeIdx];
  }

  // 设置级联矩阵
  for (uint32_t cascadeIdx = 0;
       cascadeIdx < shadowData.specific.directional.cascadeCount && cascadeIdx < MAX_CASCADES;
       cascadeIdx++)
  {
    uint32_t matrixIndex = lightIndex * MAX_CASCADES + cascadeIdx;
    m_ShadowData.directionalMatrices[matrixIndex] =
        shadowData.specific.directional.cascadeMatrices[cascadeIdx];
  }

  return true;
}
bool ShadowInstance::ProcessPointLight(std::shared_ptr<Light> light,
                                       const Transform &lightTransform,
                                       uint32_t lightIndex,
                                       std::shared_ptr<CameraInstance> cameraInstance)
{
  // 类型转换
  auto pointLight = std::dynamic_pointer_cast<PointLight>(light);
  if (!pointLight) {
    LOG_ERROR("Failed to cast to PointLight");
    return false;
  }

  // 准备阴影数据
  ShadowMapData shadowData = light->PrepareShadowData(lightIndex,
                                                      lightTransform,
                                                      cameraInstance->GetCameraTransform(),
                                                      cameraInstance->GetProjectionMatrix());
  if (!shadowData.enabled || !shadowData.isValid) {
    return false;
  }
  // 设置阴影索引
  m_ShadowData.pointShadowIndices[lightIndex] = glm::ivec4(shadowData.shadowMapIndex, 0, 0, 0);

  // 设置点光源立方体贴图矩阵（6个面）
  for (uint32_t faceIdx = 0; faceIdx < 6; faceIdx++) {
    uint32_t matrixIndex = lightIndex * 6 + faceIdx;
    m_ShadowData.pointLightMatrices[matrixIndex] =
        shadowData.specific.point.faceViewProjMatrices[faceIdx];
  }

  return true;
}
bool ShadowInstance::ProcessSpotLight(std::shared_ptr<Light> light,
                                      const Transform &lightTransform,
                                      uint32_t lightIndex,
                                      std::shared_ptr<CameraInstance> cameraInstance)
{
  // 类型转换
  auto spotLight = std::dynamic_pointer_cast<SpotLight>(light);
  if (!spotLight) {
    LOG_ERROR("Failed to cast to SpotLight");
    return false;
  }

  // 准备阴影数据
  ShadowMapData shadowData = light->PrepareShadowData(lightIndex,
                                                      lightTransform,
                                                      cameraInstance->GetCameraTransform(),
                                                      cameraInstance->GetProjectionMatrix());
  if (!shadowData.enabled || !shadowData.isValid) {
    return false;
  }
  // 设置阴影索引
  m_ShadowData.spotShadowIndices[lightIndex] = glm::ivec4(shadowData.shadowMapIndex, 0, 0, 0);

  // 设置聚光灯阴影矩阵
  m_ShadowData.spotLightMatrices[lightIndex] = shadowData.specific.spot.projectionMatrix *
                                               shadowData.specific.spot.viewMatrix;

  return true;
}
}  // namespace mite