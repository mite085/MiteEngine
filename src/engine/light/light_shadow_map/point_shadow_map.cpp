#include "point_shadow_map.h"

namespace mite {
PointShadowMap::PointShadowMap(const ShadowMapData &data)
    : ShadowMap(data), m_LastLightPosition(0.0f)
{
  // 确保数据配置正确
  if (!m_Data.enabled) {
    LOG_WARN("PointShadowMap created but not enabled");
  }

  LOG_TRACE("PointShadowMap created - near: {}, far: {}",
            m_Data.specific.point.nearPlane,
            m_Data.specific.point.farPlane);
}

ShadowMapData PointShadowMap::PrepareShadowData(const Transform &lightWorldTransform,
                                                const Transform &cameraView,
                                                const glm::mat4 &cameraProj)
{
  if (!m_Data.enabled) {
    LOG_TRACE("PointShadowMap is disabled, returning empty data");
    return ShadowMapData();
  }

  // 从世界变换矩阵提取光源位置
  glm::vec3 lightPosition = lightWorldTransform.GetPosition();

  // 检查是否需要更新阴影矩阵
  if (m_NeedsUpdate || HasLightMoved(lightPosition)) {
    CalculateCubeFaceMatrices(lightPosition);
    m_LastLightPosition = lightPosition;
    m_NeedsUpdate = false;
    m_Data.isValid = true;

    LOG_TRACE("PointShadowMap matrices updated for light at ({}, {}, {})",
              lightPosition.x,
              lightPosition.y,
              lightPosition.z);
  }

  // 返回更新后的阴影数据
  return m_Data;
}

size_t PointShadowMap::GetShadowMatrixCount() const
{
  // 点光源使用立方体贴图，固定6个面
  return 6;
}

glm::mat4 PointShadowMap::GetShadowMatrix(size_t index) const
{
  if (index >= 6) {
    LOG_ERROR("Invalid shadow matrix index for PointShadowMap: {} (max: 5)", index);
    return glm::mat4(1.0f);
  }

  if (!m_Data.isValid) {
    LOG_WARN("PointShadowMap data is not valid, returning identity matrix");
    return glm::mat4(1.0f);
  }

  return m_Data.specific.point.faceViewProjMatrices[index];
}

bool PointShadowMap::NeedsUpdate() const
{
  return m_NeedsUpdate;
}

void PointShadowMap::MarkUpdated()
{
  m_NeedsUpdate = false;
  LOG_TRACE("PointShadowMap marked as updated");
}

std::string PointShadowMap::GetShadowTypeName() const
{
  return "PointShadowMap";
}

void PointShadowMap::SetShadowRange(float nearPlane, float farPlane)
{
  if (nearPlane <= 0.0f || farPlane <= nearPlane) {
    LOG_ERROR("Invalid shadow range for PointShadowMap: near={}, far={}", nearPlane, farPlane);
    return;
  }

  m_Data.specific.point.nearPlane = nearPlane;
  m_Data.specific.point.farPlane = farPlane;
  m_NeedsUpdate = true;

  LOG_TRACE("PointShadowMap range updated - near: {}, far: {}", nearPlane, farPlane);
}

float PointShadowMap::GetNearPlane() const
{
  return m_Data.specific.point.nearPlane;
}

float PointShadowMap::GetFarPlane() const
{
  return m_Data.specific.point.farPlane;
}

void PointShadowMap::CalculateCubeFaceMatrices(const glm::vec3 &lightPosition)
{
  // 创建90度视角的透视投影矩阵（立方体贴图标准配置）
  glm::mat4 shadowProj = glm::perspective(
      glm::radians(90.0f), 1.0f, m_Data.specific.point.nearPlane, m_Data.specific.point.farPlane);

  // 立方体贴图的六个面方向定义
  // 每个面使用lookAt函数创建视图矩阵，看向对应的方向
  const std::array<glm::vec3, 6> targets = {
      glm::vec3(1.0f, 0.0f, 0.0f),   // +X 右面
      glm::vec3(-1.0f, 0.0f, 0.0f),  // -X 左面
      glm::vec3(0.0f, 1.0f, 0.0f),   // +Y 上面
      glm::vec3(0.0f, -1.0f, 0.0f),  // -Y 下面
      glm::vec3(0.0f, 0.0f, 1.0f),   // +Z 前面
      glm::vec3(0.0f, 0.0f, -1.0f)   // -Z 后面
  };

  // 每个面对应的上方向向量
  const std::array<glm::vec3, 6> ups = {
      glm::vec3(0.0f, -1.0f, 0.0f),  // +X: -Y为上
      glm::vec3(0.0f, -1.0f, 0.0f),  // -X: -Y为上
      glm::vec3(0.0f, 0.0f, 1.0f),   // +Y: +Z为上
      glm::vec3(0.0f, 0.0f, -1.0f),  // -Y: -Z为上
      glm::vec3(0.0f, -1.0f, 0.0f),  // +Z: -Y为上
      glm::vec3(0.0f, -1.0f, 0.0f)   // -Z: -Y为上
  };

  // 计算每个面的视图投影矩阵
  for (size_t i = 0; i < 6; ++i) {
    glm::mat4 shadowView = glm::lookAt(lightPosition, lightPosition + targets[i], ups[i]);
    // 存储视图投影矩阵
    m_Data.specific.point.faceViewProjMatrices[i] = shadowProj * shadowView;

    LOG_TRACE("PointShadowMap face {} matrix calculated - target: ({}, {}, {})",
              i,
              targets[i].x,
              targets[i].y,
              targets[i].z);
  }

  LOG_DEBUG("PointShadowMap all 6 face matrices calculated successfully");
}

bool PointShadowMap::HasLightMoved(const glm::vec3 &newPosition) const
{
  // 计算新旧位置的距离
  float distance = glm::distance(newPosition, m_LastLightPosition);

  // 如果移动距离超过阈值（1厘米），则认为光源移动了
  bool moved = distance > 0.01f;

  if (moved) {
    LOG_TRACE("PointShadowMap light moved - distance: {}, from ({}, {}, {}) to ({}, {}, {})",
              distance,
              m_LastLightPosition.x,
              m_LastLightPosition.y,
              m_LastLightPosition.z,
              newPosition.x,
              newPosition.y,
              newPosition.z);
  }

  return moved;
}