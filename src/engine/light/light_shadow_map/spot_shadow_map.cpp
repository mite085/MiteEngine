#include "spot_shadow_map.h"

namespace mite {
SpotShadowMap::SpotShadowMap(const ShadowMapData &data)
    : ShadowMap(data),
      m_LastLightPosition(0.0f),
      m_LastLightDirection(0.0f, 0.0f, -1.0f),
      m_Fov(45.0f),
      m_AspectRatio(1.0f) {
  // 确保数据配置正确
  if (!m_Data.enabled) {
    LOG_WARN("SpotShadowMap created but not enabled");
  }

  LOG_TRACE("SpotShadowMap created - near: {}, far: {}, fov: {}",
            m_Data.specific.spot.nearPlane, m_Data.specific.spot.farPlane,
            m_Fov);
}

ShadowMapData SpotShadowMap::PrepareShadowData(
    const uint32_t lightIndex, const Transform &lightWorldTransform,
    [[maybe_unused]] const Transform &cameraView,
    [[maybe_unused]] const glm::mat4 &cameraProj) {
  if (!m_Data.enabled) {
    LOG_TRACE("SpotShadowMap is disabled, returning empty data");
    return ShadowMapData();
  }
  // 传递序号
  m_Data.shadowMapIndex = lightIndex;

  // 从世界变换矩阵提取光源位置和方向
  glm::vec3 lightPosition = lightWorldTransform.GetPosition();
  glm::vec3 lightDirection = lightWorldTransform.GetForward();

  // 检查是否需要更新阴影矩阵
  if (m_NeedsUpdate ||
      HasLightTransformChanged(lightPosition, lightDirection)) {
    CalculateSpotLightMatrix(lightPosition, lightDirection);
    m_LastLightPosition = lightPosition;
    m_LastLightDirection = lightDirection;
    m_NeedsUpdate = false;
    m_Data.isValid = true;

    LOG_TRACE(
        "SpotShadowMap matrix updated for light at ({}, {}, {}), direction "
        "({}, {}, {})",
        lightPosition.x, lightPosition.y, lightPosition.z, lightDirection.x,
        lightDirection.y, lightDirection.z);
  }

  // 返回更新后的阴影数据
  return m_Data;
}

size_t SpotShadowMap::GetShadowMatrixCount() const {
  // 聚光灯使用单个阴影矩阵
  return 1;
}

glm::mat4 SpotShadowMap::GetShadowMatrix(size_t index) const {
  if (index != 0) {
    LOG_ERROR("Invalid shadow matrix index for SpotShadowMap: {} (must be 0)",
              index);
    return glm::mat4(1.0f);
  }

  if (!m_Data.isValid) {
    LOG_WARN("SpotShadowMap data is not valid, returning identity matrix");
    return glm::mat4(1.0f);
  }

  return m_Data.specific.spot.projectionMatrix *
         m_Data.specific.spot.viewMatrix;
}

bool SpotShadowMap::NeedsUpdate() const { return m_NeedsUpdate; }

void SpotShadowMap::MarkUpdated() {
  m_NeedsUpdate = false;
  LOG_TRACE("SpotShadowMap marked as updated");
}

std::string SpotShadowMap::GetShadowTypeName() const { return "SpotShadowMap"; }

void SpotShadowMap::SetShadowRange(float nearPlane, float farPlane) {
  if (nearPlane <= 0.0f || farPlane <= nearPlane) {
    LOG_ERROR("Invalid shadow range for SpotShadowMap: near={}, far={}",
              nearPlane, farPlane);
    return;
  }

  m_Data.specific.spot.nearPlane = nearPlane;
  m_Data.specific.spot.farPlane = farPlane;
  m_NeedsUpdate = true;

  LOG_TRACE("SpotShadowMap range updated - near: {}, far: {}", nearPlane,
            farPlane);
}

void SpotShadowMap::SetPerspectiveParams(float fov, float aspectRatio) {
  if (fov <= 0.0f || fov >= 180.0f) {
    LOG_ERROR(
        "Invalid FOV for SpotShadowMap: {} (must be between 0 and 180 degrees)",
        fov);
    return;
  }

  if (aspectRatio <= 0.0f) {
    LOG_ERROR("Invalid aspect ratio for SpotShadowMap: {}", aspectRatio);
    return;
  }

  m_Fov = fov;
  m_AspectRatio = aspectRatio;
  m_NeedsUpdate = true;

  LOG_TRACE("SpotShadowMap perspective params updated - fov: {}, aspect: {}",
            fov, aspectRatio);
}

float SpotShadowMap::GetNearPlane() const {
  return m_Data.specific.spot.nearPlane;
}

float SpotShadowMap::GetFarPlane() const {
  return m_Data.specific.spot.farPlane;
}

float SpotShadowMap::GetFov() const { return m_Fov; }

float SpotShadowMap::GetAspectRatio() const { return m_AspectRatio; }

void SpotShadowMap::CalculateSpotLightMatrix(const glm::vec3 &lightPosition,
                                             const glm::vec3 &lightDirection) {
  // 创建聚光灯的视图矩阵（看向光源方向）
  glm::vec3 target = lightPosition + lightDirection;
  m_Data.specific.spot.viewMatrix =
      glm::lookAt(lightPosition, target, glm::vec3(0.0f, 1.0f, 0.0f));

  // 创建聚光灯的透视投影矩阵
  m_Data.specific.spot.projectionMatrix = glm::perspective(
      glm::radians(m_Fov), m_AspectRatio, m_Data.specific.spot.nearPlane,
      m_Data.specific.spot.farPlane);

  LOG_TRACE(
      "SpotShadowMap matrix calculated - position: ({}, {}, {}), direction: "
      "({}, {}, {}), fov: {}",
      lightPosition.x, lightPosition.y, lightPosition.z, lightDirection.x,
      lightDirection.y, lightDirection.z, m_Fov);
}

bool SpotShadowMap::HasLightTransformChanged(
    const glm::vec3 &newPosition, const glm::vec3 &newDirection) const {
  // 计算位置变化距离
  float positionDistance = glm::distance(newPosition, m_LastLightPosition);

  // 计算方向变化角度（使用点积计算夹角）
  float directionDot = glm::dot(newDirection, m_LastLightDirection);
  float directionAngle = glm::acos(glm::clamp(directionDot, -1.0f, 1.0f)) *
                         180.0f / glm::pi<float>();

  // 如果位置移动超过阈值（1厘米）或方向旋转超过阈值（1度），则认为变换发生变化
  bool positionChanged = positionDistance > 0.01f;
  bool directionChanged = directionAngle > 1.0f;

  if (positionChanged || directionChanged) {
    LOG_TRACE(
        "SpotShadowMap light transform changed - position moved: {} m, "
        "direction rotated: {} deg",
        positionDistance, directionAngle);
  }

  return positionChanged || directionChanged;
}
}  // namespace mite