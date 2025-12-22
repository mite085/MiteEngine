#include "directional_shadow_map.h"

namespace mite {
DirectionalShadowMap::DirectionalShadowMap(const ShadowMapData &data)
    : ShadowMap(data),
      m_LastLightDirection(0.0f, 0.0f, -1.0f),
      m_LastCameraView(1.0f),
      m_LastCameraProj(1.0f) {
  // 确保数据配置正确
  if (!m_Data.enabled) {
    LOG_WARN("DirectionalShadowMap created but not enabled");
  }

  // 初始化级联分割距离
  m_Data.specific.directional.cascadeSplits = {0.0f, 0.0f, 0.0f, 0.0f};

  LOG_TRACE("DirectionalShadowMap created - cascadeCount: {}, splitLambda: {}",
            m_Data.specific.directional.cascadeCount,
            m_Data.specific.directional.splitLambda);
}

ShadowMapData DirectionalShadowMap::PrepareShadowData(
    const uint32_t lightIndex, const Transform &lightWorldTransform,
    const Transform &cameraWorldTransform, const glm::mat4 &cameraProj) {
  if (!m_Data.enabled) {
    LOG_TRACE("DirectionalShadowMap is disabled, returning empty data");
    return ShadowMapData();
  }

  // 传递序号
  m_Data.shadowMapIndex = lightIndex;

  // 从世界变换矩阵提取光源方向
  glm::vec3 lightDirection = lightWorldTransform.GetForward();

  // 获取相机视图和投影矩阵
  glm::mat4 cameraViewMatrix = cameraWorldTransform.GetViewMatrix();
  glm::mat4 cameraProjMatrix = cameraProj;

  // 检查是否需要更新阴影矩阵
  if (m_NeedsUpdate ||
      HasTransformChanged(lightDirection, cameraViewMatrix, cameraProjMatrix)) {
    // 计算级联分割距离
    CalculateCascadeSplits(cameraProjMatrix);

    // 计算级联阴影矩阵
    CalculateCascadeMatrices(lightDirection, cameraWorldTransform,
                             cameraProjMatrix);

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

size_t DirectionalShadowMap::GetShadowMatrixCount() const {
  return m_Data.specific.directional.cascadeCount;
}

glm::mat4 DirectionalShadowMap::GetShadowMatrix(size_t index) const {
  if (index >= m_Data.specific.directional.cascadeCount) {
    LOG_ERROR(
        "Invalid shadow matrix index for DirectionalShadowMap: {} (max: {})",
        index, m_Data.specific.directional.cascadeCount - 1);
    return glm::mat4(1.0f);
  }

  if (!m_Data.isValid) {
    LOG_WARN(
        "DirectionalShadowMap data is not valid, returning identity matrix");
    return glm::mat4(1.0f);
  }

  return m_Data.specific.directional.cascadeMatrices[index];
}

bool DirectionalShadowMap::NeedsUpdate() const { return m_NeedsUpdate; }

void DirectionalShadowMap::MarkUpdated() {
  m_NeedsUpdate = false;
  LOG_TRACE("DirectionalShadowMap marked as updated");
}

std::string DirectionalShadowMap::GetShadowTypeName() const {
  return "DirectionalShadowMap";
}

void DirectionalShadowMap::SetCascadeParams(unsigned int cascadeCount,
                                            float splitLambda) {
  if (cascadeCount < 1 || cascadeCount > 4) {
    LOG_ERROR(
        "Invalid cascade count for DirectionalShadowMap: {} (must be 1-4)",
        cascadeCount);
    return;
  }

  if (splitLambda < 0.0f || splitLambda > 1.0f) {
    LOG_ERROR("Invalid split lambda for DirectionalShadowMap: {} (must be 0-1)",
              splitLambda);
    return;
  }

  m_Data.specific.directional.cascadeCount = cascadeCount;
  m_Data.specific.directional.splitLambda = splitLambda;
  m_NeedsUpdate = true;

  LOG_TRACE(
      "DirectionalShadowMap cascade params updated - count: {}, lambda: {}",
      cascadeCount, splitLambda);
}

void DirectionalShadowMap::SetCascadeSplits(
    const std::array<float, MAX_CASCADES> &splits) {
  m_Data.specific.directional.cascadeSplits = splits;
  m_NeedsUpdate = true;

  LOG_TRACE("DirectionalShadowMap cascade splits updated");
}

unsigned int DirectionalShadowMap::GetCascadeCount() const {
  return m_Data.specific.directional.cascadeCount;
}

float DirectionalShadowMap::GetSplitLambda() const {
  return m_Data.specific.directional.splitLambda;
}

const std::array<float, MAX_CASCADES> &DirectionalShadowMap::GetCascadeSplits()
    const {
  return m_Data.specific.directional.cascadeSplits;
}

void DirectionalShadowMap::CalculateCascadeSplits(const glm::mat4 &cameraProj) {
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
  // 索引从1开始（i = 0时，计算结果为0级级联的近平面）
  for (unsigned int i = 1; i <= cascadeCount; ++i) {
    float fraction = static_cast<float>(i) / cascadeCount;
    float logSplit = nearPlane * std::pow(farPlane / nearPlane, fraction);
    float uniformSplit = nearPlane + (farPlane - nearPlane) * fraction;

    // 混合对数和均匀分割（仅记录每个级联的远平面距离）
    m_Data.specific.directional.cascadeSplits[i - 1] =
        splitLambda * logSplit + (1.0f - splitLambda) * uniformSplit;
  }

  LOG_TRACE(
      "DirectionalShadowMap cascade splits calculated - near: {}, far: {}, "
      "splits: [{}, {}, {}, "
      "{}]",
      nearPlane, farPlane, m_Data.specific.directional.cascadeSplits[0],
      m_Data.specific.directional.cascadeSplits[1],
      m_Data.specific.directional.cascadeSplits[2],
      m_Data.specific.directional.cascadeSplits[3]);
}

void DirectionalShadowMap::CalculateCascadeMatrices(
    const glm::vec3 &lightDirection, const Transform &cameraWorldTransform,
    const glm::mat4 &cameraProj) {
  unsigned int cascadeCount = m_Data.specific.directional.cascadeCount;

  // 从投影矩阵提取相机近远平面
  float cameraNearPlane = 0.1f;
  [[maybe_unused]] float cameraFarPlane = 1000.0f;
  if (cameraProj[3][3] == 0.0f) {  // 透视投影
    cameraNearPlane = cameraProj[3][2] / (cameraProj[2][2] - 1.0f);
    cameraFarPlane = cameraProj[3][2] / (cameraProj[2][2] + 1.0f);
  }

  // 从Transform获取相机参数
  glm::vec3 cameraPos = cameraWorldTransform.GetPosition();
  glm::vec3 cameraForward =
      cameraWorldTransform.GetForward();  // 相机看向的方向

  // 调试信息：相机和光源基本信息
  LOG_TRACE("DirectionalShadowMap: Calculating {} cascades", cascadeCount);
  LOG_TRACE(
      "Camera: pos=({:.1f},{:.1f},{:.1f}), forward=({:.3f},{:.3f},{:.3f})",
      cameraPos.x, cameraPos.y, cameraPos.z, cameraForward.x, cameraForward.y,
      cameraForward.z);
  LOG_TRACE("Light direction: ({:.3f},{:.3f},{:.3f})", lightDirection.x,
            lightDirection.y, lightDirection.z);

  for (unsigned int i = 0; i < cascadeCount; ++i) {
    // 获取当前级联的近远平面
    float nearSplit = (i == 0)
                          ? cameraNearPlane
                          : m_Data.specific.directional.cascadeSplits[i - 1];
    float farSplit = m_Data.specific.directional.cascadeSplits[i];

    LOG_TRACE("Cascade {}: near={:.1f}, far={:.1f}", i, nearSplit, farSplit);

    // 1. 计算级联中心点（在相机视线上）
    // 使用级联的中间深度作为中心点深度
    float centerDepth = (nearSplit + farSplit) * 0.5f;
    glm::vec3 cascadeCenter = cameraPos + cameraForward * centerDepth;

    LOG_TRACE("  Center depth: {:.1f}, world center: ({:.1f},{:.1f},{:.1f})",
              centerDepth, cascadeCenter.x, cascadeCenter.y, cascadeCenter.z);

    // 2. 计算正交投影的大小
    // 基于级联的深度范围确定正交投影大小
    float cascadeDepth = farSplit - nearSplit;

    // 使用级联深度的倍数作为正交投影大小
    // 这个倍数需要根据场景调整，2.0是一个合理的起始值
    float orthoSize = cascadeDepth * 2.0f;

    // 确保最小尺寸，避免数值问题
    orthoSize = glm::max(orthoSize, 5.0f);

    LOG_TRACE("  Cascade depth: {:.1f}, ortho size: {:.1f}", cascadeDepth,
              orthoSize);

    // 3. 选择光源视图的上向量
    // 避免上向量与光源方向平行（会导致视图矩阵无效）
    glm::vec3 up = Transform::GetWorldUp();  // 默认使用世界向上方向

    // 检查光源方向是否与上向量接近平行
    float dotWithUp = glm::abs(glm::dot(lightDirection, up));
    if (dotWithUp > 0.9f) {
      // 如果接近平行，使用替代的上向量（Z轴）
      up = glm::vec3(0.0f, 0.0f, 1.0f);
      LOG_TRACE("  Using alternative up vector: (0,0,1)");
    }

    // 4. 确定光源位置
    // 光源应该在级联中心的后方，看向级联中心
    // 使用正交投影大小的倍数作为光源距离
    float lightDistance = orthoSize * 2.0f;
    glm::vec3 lightPosition = cascadeCenter - lightDirection * lightDistance;

    LOG_TRACE("  Light position: ({:.1f},{:.1f},{:.1f}), distance: {:.1f}",
              lightPosition.x, lightPosition.y, lightPosition.z, lightDistance);

    // 5. 创建光源视图矩阵
    // 光源从lightPosition看向cascadeCenter，使用up作为上方向
    glm::mat4 lightView = glm::lookAt(lightPosition, cascadeCenter, up);

    // 6. 设置正交投影参数
    // 使用以级联中心为中心的正交投影
    float halfSize = orthoSize * 0.5f;

    // 设置Z范围（到光源的距离）
    // zNear: 最近裁剪平面距离（正值）
    // zFar: 最远裁剪平面距离（正值）
    // 对于方向光阴影，我们需要包含从光源位置到级联中心及其后方的所有几何体
    float zNear = 0.1f;                 // 最小距离
    float zFar = lightDistance * 2.0f;  // 最大距离（包含级联中心后方的几何体）

    // 确保zFar > zNear
    if (zFar <= zNear) {
      zFar = zNear + 100.0f;
    }

    LOG_TRACE(
        "  Ortho bounds: X=[{:.1f},{:.1f}], Y=[{:.1f},{:.1f}], "
        "Z=[{:.1f},{:.1f}]",
        -halfSize, halfSize, -halfSize, halfSize, zNear, zFar);

    // 7. 创建正交投影矩阵
    glm::mat4 lightProjection = glm::ortho(-halfSize,
                                           halfSize,  // X范围
                                           -halfSize,
                                           halfSize,  // Y范围
                                           zNear,
                                           zFar  // Z范围（到光源的距离）
    );

    // 8. 应用阴影贴图稳定化
    glm::mat4 shadowMatrix = lightProjection * lightView;
    shadowMatrix = StabilizeShadowMatrix(
        shadowMatrix, static_cast<float>(ShadowQuality::MEDIUM));

    // 9. 存储稳定化后的矩阵
    m_Data.specific.directional.cascadeMatrices[i] = shadowMatrix;

    // 10. 验证矩阵（调试用）
    ValidateCascadeMatrix(i, cameraPos, cameraForward, nearSplit, farSplit,
                          cameraProj,
                          m_Data.specific.directional.cascadeMatrices[i]);

    LOG_TRACE("  Cascade {} matrix calculated successfully", i);
  }

  LOG_TRACE("DirectionalShadowMap: All cascade matrices calculated");
}

glm::mat4 DirectionalShadowMap::StabilizeShadowMatrix(
    const glm::mat4 &shadowMatrix, float shadowMapResolution) {
  // 将世界空间点转换到阴影贴图空间
  glm::vec4 shadowOrigin = shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  shadowOrigin *= shadowMapResolution / 2.0f;

  // 计算纹素对齐的偏移
  glm::vec2 roundedOrigin =
      glm::round(glm::vec2(shadowOrigin.x, shadowOrigin.y));
  glm::vec2 roundOffset =
      roundedOrigin - glm::vec2(shadowOrigin.x, shadowOrigin.y);
  roundOffset /= (shadowMapResolution / 2.0f);

  // 创建偏移矩阵
  glm::mat4 offsetMatrix = glm::translate(
      glm::mat4(1.0f), glm::vec3(roundOffset.x, roundOffset.y, 0.0f));

  // 返回稳定化后的矩阵
  return offsetMatrix * shadowMatrix;
}

void DirectionalShadowMap::ValidateCascadeMatrix(
    unsigned int cascadeIndex, const glm::vec3 &cameraPos,
    const glm::vec3 &cameraForward, float nearSplit, float farSplit,
    [[maybe_unused]] const glm::mat4 &cameraProj,
    const glm::mat4 &shadowMatrix) const {
  // 简单验证：检查几个关键点是否在阴影投影范围内

  // 计算视锥体中心线上的几个点
  std::vector<glm::vec3> testPoints;

  // 近平面中心点
  testPoints.push_back(cameraPos + cameraForward * nearSplit);
  // 中间点
  testPoints.push_back(cameraPos +
                       cameraForward * ((nearSplit + farSplit) * 0.5f));
  // 远平面中心点
  testPoints.push_back(cameraPos + cameraForward * farSplit);

  int outOfBounds = 0;
  for (size_t i = 0; i < testPoints.size(); ++i) {
    glm::vec4 projPoint = shadowMatrix * glm::vec4(testPoints[i], 1.0f);
    projPoint /= projPoint.w;

    // 检查是否在标准设备坐标范围内[-1, 1]
    if (projPoint.x < -1.01f || projPoint.x > 1.01f || projPoint.y < -1.01f ||
        projPoint.y > 1.01f || projPoint.z < -1.01f || projPoint.z > 1.01f) {
      outOfBounds++;
      LOG_WARN("Cascade {} test point {} out of bounds: ({:.2f},{:.2f},{:.2f})",
               cascadeIndex, i, projPoint.x, projPoint.y, projPoint.z);
    }
  }

  if (outOfBounds == 0) {
    LOG_TRACE("Cascade {} validation passed: all test points in bounds",
              cascadeIndex);
  } else {
    LOG_WARN("Cascade {} validation: {}/{} points out of bounds", cascadeIndex,
             outOfBounds, testPoints.size());
  }
}

std::array<glm::vec3, 8> DirectionalShadowMap::CalculateFrustumCornersGeometric(
    float nearPlane, float farPlane, const glm::vec3 &cameraPos,
    const glm::vec3 &cameraForward, const glm::vec3 &cameraUp,
    const glm::vec3 &cameraRight, const glm::mat4 &cameraProj) const {
  // 从投影矩阵提取FOV和宽高比
  float fovY, aspect;

  if (cameraProj[3][3] == 0.0f) {  // 透视投影
    fovY = 2.0f * atan(1.0f / cameraProj[1][1]);
    aspect = cameraProj[1][1] / cameraProj[0][0];
  } else {                       // 正交投影
    fovY = glm::radians(60.0f);  // 默认值
    aspect = 1.0f;
  }

  // 计算近平面和远平面的半高、半宽
  float tanHalfFovY = tan(fovY * 0.5f);

  float nearHalfHeight = tanHalfFovY * nearPlane;
  float nearHalfWidth = nearHalfHeight * aspect;

  float farHalfHeight = tanHalfFovY * farPlane;
  float farHalfWidth = farHalfHeight * aspect;

  // 计算角点
  std::array<glm::vec3, 8> corners;

  // 近平面角点
  corners[0] = cameraPos + cameraForward * nearPlane -
               cameraUp * nearHalfHeight - cameraRight * nearHalfWidth;
  corners[1] = cameraPos + cameraForward * nearPlane -
               cameraUp * nearHalfHeight + cameraRight * nearHalfWidth;
  corners[2] = cameraPos + cameraForward * nearPlane +
               cameraUp * nearHalfHeight + cameraRight * nearHalfWidth;
  corners[3] = cameraPos + cameraForward * nearPlane +
               cameraUp * nearHalfHeight - cameraRight * nearHalfWidth;

  // 远平面角点
  corners[4] = cameraPos + cameraForward * farPlane - cameraUp * farHalfHeight -
               cameraRight * farHalfWidth;
  corners[5] = cameraPos + cameraForward * farPlane - cameraUp * farHalfHeight +
               cameraRight * farHalfWidth;
  corners[6] = cameraPos + cameraForward * farPlane + cameraUp * farHalfHeight +
               cameraRight * farHalfWidth;
  corners[7] = cameraPos + cameraForward * farPlane + cameraUp * farHalfHeight -
               cameraRight * farHalfWidth;

  // 调试输出
  LOG_DEBUG("Frustum corners for near={:.1f}, far={:.1f}:", nearPlane,
            farPlane);
  for (int i = 0; i < 8; ++i) {
    LOG_DEBUG("  Corner {}: ({:.1f}, {:.1f}, {:.1f})", i, corners[i].x,
              corners[i].y, corners[i].z);
  }

  return corners;
}

bool DirectionalShadowMap::HasTransformChanged(
    const glm::vec3 &newLightDirection, const glm::mat4 &newCameraView,
    const glm::mat4 &newCameraProj) const {
  // 计算方向变化角度
  float directionDot = glm::dot(newLightDirection, m_LastLightDirection);
  float directionAngle = glm::acos(glm::clamp(directionDot, -1.0f, 1.0f)) *
                         180.0f / glm::pi<float>();

  // 计算相机矩阵变化（使用矩阵差的Frobenius范数）
  glm::mat4 cameraViewDiff = newCameraView - m_LastCameraView;
  glm::mat4 cameraProjDiff = newCameraProj - m_LastCameraProj;

  float viewChange =
      glm::length(glm::vec4(cameraViewDiff[0]) + glm::vec4(cameraViewDiff[1]) +
                  glm::vec4(cameraViewDiff[2]) + glm::vec4(cameraViewDiff[3]));
  float projChange =
      glm::length(glm::vec4(cameraProjDiff[0]) + glm::vec4(cameraProjDiff[1]) +
                  glm::vec4(cameraProjDiff[2]) + glm::vec4(cameraProjDiff[3]));

  // 如果光源方向旋转超过阈值（1度）或相机矩阵变化超过阈值，则认为变换发生变化
  bool directionChanged = directionAngle > 1.0f;
  bool cameraChanged = (viewChange > 0.01f) || (projChange > 0.01f);

  if (directionChanged || cameraChanged) {
    LOG_TRACE(
        "DirectionalShadowMap transform changed - direction rotated: {}°, "
        "camera changed: {}",
        directionAngle, cameraChanged);
  }

  return directionChanged || cameraChanged;
}
}  // namespace mite