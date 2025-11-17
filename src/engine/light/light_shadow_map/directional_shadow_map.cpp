#include "directional_shadow_map.h"

namespace mite {

DirectionalShadowMap::DirectionalShadowMap(const ShadowMapData &data)
    : ShadowMap(data),
      m_LastLightDirection(0.0f, 0.0f, -1.0f),
      m_LastCameraView(1.0f),
      m_LastCameraProj(1.0f)
{
  // 确保数据配置正确
  if (!m_Data.enabled) {
    LOG_WARN("DirectionalShadowMap created but not enabled");
  }

  // 初始化级联分割距离
  m_Data.specific.directional.cascadeSplits = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  LOG_TRACE("DirectionalShadowMap created - cascadeCount: {}, splitLambda: {}",
            m_Data.specific.directional.cascadeCount,
            m_Data.specific.directional.splitLambda);
}

ShadowMapData DirectionalShadowMap::PrepareShadowData(const uint32_t lightIndex,
                                                      const Transform &lightWorldTransform,
                                                      const Transform &cameraView,
                                                      const glm::mat4 &cameraProj)
{
  if (!m_Data.enabled) {
    LOG_TRACE("DirectionalShadowMap is disabled, returning empty data");
    return ShadowMapData();
  }

  // 传递序号
  m_Data.shadowMapIndex = lightIndex;

  // 从世界变换矩阵提取光源方向
  glm::vec3 lightDirection = lightWorldTransform.GetForward();

  // 获取相机视图和投影矩阵
  glm::mat4 cameraViewMatrix = cameraView.GetLocalMatrix();
  glm::mat4 cameraProjMatrix = cameraProj;

  // 检查是否需要更新阴影矩阵
  if (m_NeedsUpdate || HasTransformChanged(lightDirection, cameraViewMatrix, cameraProjMatrix)) {
    // 计算级联分割距离
    CalculateCascadeSplits(cameraProjMatrix);

    // 计算级联阴影矩阵
    CalculateCascadeMatrices(lightDirection, cameraViewMatrix, cameraProjMatrix);

    m_LastLightDirection = lightDirection;
    m_LastCameraView = cameraViewMatrix;
    m_LastCameraProj = cameraProjMatrix;
    m_NeedsUpdate = false;
    m_Data.isValid = true;

    LOG_TRACE("DirectionalShadowMap matrices updated for {} cascades",
              m_Data.specific.directional.cascadeCount);
  }

  // 返回更新后的阴影数据
  return m_Data;
}

size_t DirectionalShadowMap::GetShadowMatrixCount() const
{
  return m_Data.specific.directional.cascadeCount;
}

glm::mat4 DirectionalShadowMap::GetShadowMatrix(size_t index) const
{
  if (index >= m_Data.specific.directional.cascadeCount) {
    LOG_ERROR("Invalid shadow matrix index for DirectionalShadowMap: {} (max: {})",
              index,
              m_Data.specific.directional.cascadeCount - 1);
    return glm::mat4(1.0f);
  }

  if (!m_Data.isValid) {
    LOG_WARN("DirectionalShadowMap data is not valid, returning identity matrix");
    return glm::mat4(1.0f);
  }

  return m_Data.specific.directional.cascadeMatrices[index];
}

bool DirectionalShadowMap::NeedsUpdate() const
{
  return m_NeedsUpdate;
}

void DirectionalShadowMap::MarkUpdated()
{
  m_NeedsUpdate = false;
  LOG_TRACE("DirectionalShadowMap marked as updated");
}

std::string DirectionalShadowMap::GetShadowTypeName() const
{
  return "DirectionalShadowMap";
}

void DirectionalShadowMap::SetCascadeParams(unsigned int cascadeCount, float splitLambda)
{
  if (cascadeCount < 1 || cascadeCount > 4) {
    LOG_ERROR("Invalid cascade count for DirectionalShadowMap: {} (must be 1-4)", cascadeCount);
    return;
  }

  if (splitLambda < 0.0f || splitLambda > 1.0f) {
    LOG_ERROR("Invalid split lambda for DirectionalShadowMap: {} (must be 0-1)", splitLambda);
    return;
  }

  m_Data.specific.directional.cascadeCount = cascadeCount;
  m_Data.specific.directional.splitLambda = splitLambda;
  m_NeedsUpdate = true;

  LOG_TRACE("DirectionalShadowMap cascade params updated - count: {}, lambda: {}",
            cascadeCount,
            splitLambda);
}

void DirectionalShadowMap::SetCascadeSplits(const std::array<float, 5> &splits)
{
  m_Data.specific.directional.cascadeSplits = splits;
  m_NeedsUpdate = true;

  LOG_TRACE("DirectionalShadowMap cascade splits updated");
}

unsigned int DirectionalShadowMap::GetCascadeCount() const
{
  return m_Data.specific.directional.cascadeCount;
}

float DirectionalShadowMap::GetSplitLambda() const
{
  return m_Data.specific.directional.splitLambda;
}

const std::array<float, 5> &DirectionalShadowMap::GetCascadeSplits() const
{
  return m_Data.specific.directional.cascadeSplits;
}

void DirectionalShadowMap::CalculateCascadeSplits(const glm::mat4 &cameraProj)
{
  // 从投影矩阵提取近远平面距离（假设使用透视投影）
  float nearPlane = 0.1f;
  float farPlane = 100.0f;

  // 尝试从投影矩阵提取近远平面
  // 注意：这假设使用标准的透视投影矩阵
  if (cameraProj[3][3] == 0.0f) {  // 透视投影
    nearPlane = cameraProj[3][2] / (cameraProj[2][2] - 1.0f);
    farPlane = cameraProj[3][2] / (cameraProj[2][2] + 1.0f);
  }

  unsigned int cascadeCount = m_Data.specific.directional.cascadeCount;
  float splitLambda = m_Data.specific.directional.splitLambda;

  // 计算级联分割距离（使用对数分割策略）
  for (unsigned int i = 0; i <= cascadeCount; ++i) {
    float fraction = static_cast<float>(i) / cascadeCount;
    float logSplit = nearPlane * std::pow(farPlane / nearPlane, fraction);
    float uniformSplit = nearPlane + (farPlane - nearPlane) * fraction;

    // 混合对数和均匀分割
    m_Data.specific.directional.cascadeSplits[i] = splitLambda * logSplit +
                                                   (1.0f - splitLambda) * uniformSplit;
  }

  LOG_TRACE(
      "DirectionalShadowMap cascade splits calculated - near: {}, far: {}, splits: [{}, {}, {}, "
      "{}, {}]",
      nearPlane,
      farPlane,
      m_Data.specific.directional.cascadeSplits[0],
      m_Data.specific.directional.cascadeSplits[1],
      m_Data.specific.directional.cascadeSplits[2],
      m_Data.specific.directional.cascadeSplits[3],
      m_Data.specific.directional.cascadeSplits[4]);
}

void DirectionalShadowMap::CalculateCascadeMatrices(const glm::vec3 &lightDirection,
                                                    const glm::mat4 &cameraView,
                                                    const glm::mat4 &cameraProj)
{
  unsigned int cascadeCount = m_Data.specific.directional.cascadeCount;

  for (unsigned int i = 0; i < cascadeCount; ++i) {
    float nearSplit = m_Data.specific.directional.cascadeSplits[i];
    float farSplit = m_Data.specific.directional.cascadeSplits[i + 1];

    // 计算当前级联的视锥体角点
    auto frustumCorners = CalculateFrustumCorners(nearSplit, farSplit, cameraView, cameraProj);

    // 计算视锥体包围盒的中心
    glm::vec3 center(0.0f);
    for (const auto &corner : frustumCorners) {
      center += corner;
    }
    center /= 8.0f;

    // 创建光源视图矩阵
    glm::mat4 lightView = glm::lookAt(
        center - lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));

    // 计算视锥体角点在光源空间中的包围盒
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto &corner : frustumCorners) {
      glm::vec4 trf = lightView * glm::vec4(corner, 1.0f);
      minX = std::min(minX, trf.x);
      maxX = std::max(maxX, trf.x);
      minY = std::min(minY, trf.y);
      maxY = std::max(maxY, trf.y);
      minZ = std::min(minZ, trf.z);
      maxZ = std::max(maxZ, trf.z);
    }

    // 创建光源投影矩阵（正交投影）
    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    // 存储级联矩阵
    m_Data.specific.directional.cascadeMatrices[i] = lightProjection * lightView;

    LOG_TRACE(
        "DirectionalShadowMap cascade {} matrix calculated - near: {}, far: {}, bounds: [{}, "
        "{}]x[{}, {}]x[{}, {}]",
        i,
        nearSplit,
        farSplit,
        minX,
        maxX,
        minY,
        maxY,
        minZ,
        maxZ);
  }
}

std::array<glm::vec3, 8> DirectionalShadowMap::CalculateFrustumCorners(
    float nearPlane,
    float farPlane,
    const glm::mat4 &cameraView,
    const glm::mat4 &cameraProj) const
{
  // 计算视图投影矩阵的逆矩阵
  glm::mat4 invViewProj = glm::inverse(cameraProj * cameraView);

  std::array<glm::vec3, 8> corners;

  // 定义NDC空间的8个角点
  const std::array<glm::vec4, 8> ndcCorners = {// 近平面
                                               glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
                                               glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
                                               glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),
                                               glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
                                               // 远平面
                                               glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
                                               glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
                                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                               glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f)};

  // 将NDC坐标转换到世界空间
  for (size_t i = 0; i < 8; ++i) {
    glm::vec4 worldPos = invViewProj * ndcCorners[i];
    corners[i] = glm::vec3(worldPos) / worldPos.w;
  }

  return corners;
}

bool DirectionalShadowMap::HasTransformChanged(const glm::vec3 &newLightDirection,
                                               const glm::mat4 &newCameraView,
                                               const glm::mat4 &newCameraProj) const
{
  // 计算方向变化角度
  float directionDot = glm::dot(newLightDirection, m_LastLightDirection);
  float directionAngle = glm::acos(glm::clamp(directionDot, -1.0f, 1.0f)) * 180.0f /
                         glm::pi<float>();

  // 计算相机矩阵变化（使用矩阵差的Frobenius范数）
  glm::mat4 cameraViewDiff = newCameraView - m_LastCameraView;
  glm::mat4 cameraProjDiff = newCameraProj - m_LastCameraProj;

  float viewChange = glm::length(glm::vec4(cameraViewDiff[0]) + glm::vec4(cameraViewDiff[1]) +
                                 glm::vec4(cameraViewDiff[2]) + glm::vec4(cameraViewDiff[3]));
  float projChange = glm::length(glm::vec4(cameraProjDiff[0]) + glm::vec4(cameraProjDiff[1]) +
                                 glm::vec4(cameraProjDiff[2]) + glm::vec4(cameraProjDiff[3]));

  // 如果光源方向旋转超过阈值（1度）或相机矩阵变化超过阈值，则认为变换发生变化
  bool directionChanged = directionAngle > 1.0f;
  bool cameraChanged = (viewChange > 0.01f) || (projChange > 0.01f);

  if (directionChanged || cameraChanged) {
    LOG_TRACE(
        "DirectionalShadowMap transform changed - direction rotated: {}°, camera changed: {}",
        directionAngle,
        cameraChanged);
  }

  return directionChanged || cameraChanged;
}

}  // namespace mite
