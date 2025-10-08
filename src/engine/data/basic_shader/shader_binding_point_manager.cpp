#include "shader_binding_point_manager.h"

namespace mite {
BindingPointManager &BindingPointManager::Get()
{
  static BindingPointManager instance;
  return instance;
}

BindingPointManager::BindingPointManager()
{
  // 初始化各类型的下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(ShaderBufferResourceType::Count); ++i) {
    ShaderBufferResourceType type = static_cast<ShaderBufferResourceType>(i);
    m_NextBindingPoints[i] = GetRangeStart(type);
  }

  // 验证范围设置
  ValidateRanges();

  LOG_INFO("BindingPointManager initialized with {} total binding points",
           ShaderBufferBindingRanges::TOTAL_BINDING_POINTS);
}

void BindingPointManager::PreallocateCommonResources()
{
  // 预分配常用UBO资源
  m_CameraUBOBinding = AllocateBindingPoint(ShaderBufferResourceType::CameraUBO,
                                            ShaderBufferResourceNames::CAMERA_UBO);
  m_MaterialUBOBinding = AllocateBindingPoint(ShaderBufferResourceType::MaterialUBO,
                                              ShaderBufferResourceNames::MATERIAL_UBO);
  m_ModelUBOBinding = AllocateBindingPoint(ShaderBufferResourceType::ModelUBO,
                                           ShaderBufferResourceNames::MODEL_UBO);

  // 预分配常用SSBO资源
  m_LightSSBOBinding = AllocateBindingPoint(ShaderBufferResourceType::LightSSBO,
                                            ShaderBufferResourceNames::LIGHT_SSBO);

  // 预分配常用纹理资源
  m_BaseColorTextureBinding = AllocateBindingPoint(ShaderBufferResourceType::BaseColorTexture,
                                                   ShaderBufferResourceNames::BASE_COLOR_TEXTURE);
  m_NormalTextureBinding = AllocateBindingPoint(ShaderBufferResourceType::NormalTexture,
                                                ShaderBufferResourceNames::NORMAL_TEXTURE);
  m_MetallicRoughnessTextureBinding = AllocateBindingPoint(
      ShaderBufferResourceType::MetallicRoughnessTexture,
      ShaderBufferResourceNames::METALLIC_ROUGHNESS_TEXTURE);
  m_EmissiveTextureBinding = AllocateBindingPoint(ShaderBufferResourceType::EmissiveTexture,
                                                  ShaderBufferResourceNames::EMISSIVE_TEXTURE);
  m_OcclusionTextureBinding = AllocateBindingPoint(ShaderBufferResourceType::OcclusionTexture,
                                                   ShaderBufferResourceNames::OCCLUSION_TEXTURE);

  m_ShadowMapBinding = AllocateBindingPoint(ShaderBufferResourceType::ShadowMap,
                                            ShaderBufferResourceNames::SHADOW_MAP);
  m_EnvironmentMapBinding = AllocateBindingPoint(ShaderBufferResourceType::EnvironmentMap,
                                                 ShaderBufferResourceNames::ENVIRONMENT_MAP);

  LOG_INFO("Preallocated common resources:");
  LOG_INFO("  UBOs: Camera={}, Material={}, Model={}",
           m_CameraUBOBinding,
           m_MaterialUBOBinding,
           m_ModelUBOBinding);
  LOG_INFO("  SSBOs: Lights={}", m_LightSSBOBinding);
  LOG_INFO("  Textures: BaseColor={}, Normal={}, MR={}, Emissive={}, Occlusion={}",
           m_BaseColorTextureBinding,
           m_NormalTextureBinding,
           m_MetallicRoughnessTextureBinding,
           m_EmissiveTextureBinding,
           m_OcclusionTextureBinding);
  LOG_INFO("  Environment: Shadow={}, EnvMap={}", m_ShadowMapBinding, m_EnvironmentMapBinding);
}

uint32_t BindingPointManager::AllocateBindingPoint(ShaderBufferResourceType type,
                                                   const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  size_t typeIndex = static_cast<size_t>(type);
  uint32_t rangeStart = GetRangeStart(type);
  uint32_t rangeCount = GetRangeCount(type);
  uint32_t rangeEnd = rangeStart + rangeCount;

  // 从该类型的下一个可用点开始查找
  uint32_t startPoint = m_NextBindingPoints[typeIndex].load();
  uint32_t currentPoint = startPoint;

  do {
    // 检查当前点是否可用
    if (currentPoint < rangeEnd && !m_AllocatedPoints.test(currentPoint)) {
      // 分配绑定点
      m_AllocatedPoints.set(currentPoint);
      m_ResourceTypes[currentPoint] = type;

      // 更新下一个可用点
      m_NextBindingPoints[typeIndex] = (currentPoint + 1) % rangeEnd;
      if (m_NextBindingPoints[typeIndex] < rangeStart) {
        m_NextBindingPoints[typeIndex] = rangeStart;
      }

      LOG_DEBUG(
          "Allocated binding point {} for {}[{}]", currentPoint, name, static_cast<int>(type));
      return currentPoint;
    }

    // 尝试下一个点
    currentPoint++;
    if (currentPoint >= rangeEnd) {
      currentPoint = rangeStart;
    }
  } while (currentPoint != startPoint);  // 循环回到起点时停止

  // 没有可用绑定点
  LOG_ERROR("Failed to allocate binding point for type {}: no available points in range [{}-{})",
            static_cast<int>(type),
            rangeStart,
            rangeEnd);
  return UINT32_MAX;
}

void BindingPointManager::ReleaseBindingPoint(uint32_t bindingPoint)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!IsValidBindingPoint(bindingPoint)) {
    LOG_WARN("Attempted to release invalid binding point: {}", bindingPoint);
    return;
  }

  if (!m_AllocatedPoints.test(bindingPoint)) {
    LOG_WARN("Attempted to release unallocated binding point: {}", bindingPoint);
    return;
  }

  // 释放绑定点
  m_AllocatedPoints.reset(bindingPoint);
  m_ResourceTypes.erase(bindingPoint);

  LOG_DEBUG("Released binding point: {}", bindingPoint);
}

ShaderBufferResourceType BindingPointManager::GetResourceType(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_ResourceTypes.find(bindingPoint);
  if (it != m_ResourceTypes.end()) {
    return it->second;
  }
  return ShaderBufferResourceType::Count;  // 无效类型
}

bool BindingPointManager::IsBindingPointAllocated(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return IsValidBindingPoint(bindingPoint) && m_AllocatedPoints.test(bindingPoint);
}

size_t BindingPointManager::GetAllocatedCount() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_AllocatedPoints.count();
}

void BindingPointManager::Reset()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  m_AllocatedPoints.reset();
  m_ResourceTypes.clear();

  // 重置各类型的下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(ShaderBufferResourceType::Count); ++i) {
    ShaderBufferResourceType type = static_cast<ShaderBufferResourceType>(i);
    m_NextBindingPoints[i] = GetRangeStart(type);
  }

  // 重置预分配的资源
  m_CameraUBOBinding = UINT32_MAX;
  m_MaterialUBOBinding = UINT32_MAX;
  m_ModelUBOBinding = UINT32_MAX;
  m_LightSSBOBinding = UINT32_MAX;
  m_BaseColorTextureBinding = UINT32_MAX;
  m_NormalTextureBinding = UINT32_MAX;
  m_MetallicRoughnessTextureBinding = UINT32_MAX;
  m_EmissiveTextureBinding = UINT32_MAX;
  m_OcclusionTextureBinding = UINT32_MAX;
  m_ShadowMapBinding = UINT32_MAX;
  m_EnvironmentMapBinding = UINT32_MAX;

  LOG_INFO("BindingPointManager reset");
}

// ---- 内部方法 ----
uint32_t BindingPointManager::GetRangeStart(ShaderBufferResourceType type) const
{
  switch (type) {
    // UBO类型 (0-15)
    case ShaderBufferResourceType::CameraUBO:
    case ShaderBufferResourceType::MaterialUBO:
    case ShaderBufferResourceType::ModelUBO:
    case ShaderBufferResourceType::SceneUBO:
      return ShaderBufferBindingRanges::UBO_START;
    // SSBO类型 (16-31)
    case ShaderBufferResourceType::LightSSBO:
    case ShaderBufferResourceType::InstanceSSBO:
    case ShaderBufferResourceType::BoneSSBO:
      return ShaderBufferBindingRanges::SSBO_START;
    // 纹理类型 (32-95)
    case ShaderBufferResourceType::BaseColorTexture:
    case ShaderBufferResourceType::NormalTexture:
    case ShaderBufferResourceType::MetallicRoughnessTexture:
    case ShaderBufferResourceType::EmissiveTexture:
    case ShaderBufferResourceType::OcclusionTexture:
    case ShaderBufferResourceType::ShadowMap:
    case ShaderBufferResourceType::EnvironmentMap:
    case ShaderBufferResourceType::BRDFLUT:
    case ShaderBufferResourceType::IrradianceMap:
    case ShaderBufferResourceType::PrefilterMap:
    case ShaderBufferResourceType::ColorGradingLUT:
    case ShaderBufferResourceType::BloomTexture:
    case ShaderBufferResourceType::SSAOTexture:
    case ShaderBufferResourceType::CustomTexture0:
      return ShaderBufferBindingRanges::TEXTURE_START;
    default:
      LOG_ERROR("Unknown resource type: {}", static_cast<int>(type));
      return 0;
  }
}
uint32_t BindingPointManager::GetRangeCount(ShaderBufferResourceType type) const
{
  switch (type) {
    // UBO类型 (16个绑定点)
    case ShaderBufferResourceType::CameraUBO:
    case ShaderBufferResourceType::MaterialUBO:
    case ShaderBufferResourceType::ModelUBO:
    case ShaderBufferResourceType::SceneUBO:
      return ShaderBufferBindingRanges::UBO_COUNT;
    // SSBO类型 (16个绑定点)
    case ShaderBufferResourceType::LightSSBO:
    case ShaderBufferResourceType::InstanceSSBO:
    case ShaderBufferResourceType::BoneSSBO:
      return ShaderBufferBindingRanges::SSBO_COUNT;
    // 纹理类型 (64个绑定点)
    case ShaderBufferResourceType::BaseColorTexture:
    case ShaderBufferResourceType::NormalTexture:
    case ShaderBufferResourceType::MetallicRoughnessTexture:
    case ShaderBufferResourceType::EmissiveTexture:
    case ShaderBufferResourceType::OcclusionTexture:
    case ShaderBufferResourceType::ShadowMap:
    case ShaderBufferResourceType::EnvironmentMap:
    case ShaderBufferResourceType::BRDFLUT:
    case ShaderBufferResourceType::IrradianceMap:
    case ShaderBufferResourceType::PrefilterMap:
    case ShaderBufferResourceType::ColorGradingLUT:
    case ShaderBufferResourceType::BloomTexture:
    case ShaderBufferResourceType::SSAOTexture:
    case ShaderBufferResourceType::CustomTexture0:
      return ShaderBufferBindingRanges::TEXTURE_COUNT;
    default:
      LOG_ERROR("Unknown resource type: {}", static_cast<int>(type));
      return 0;
  }
}

bool BindingPointManager::IsValidBindingPoint(uint32_t point) const
{
  return point < ShaderBufferBindingRanges::TOTAL_BINDING_POINTS;
}

void BindingPointManager::ValidateRanges() const
{
  // 检查范围不重叠
  static_assert(ShaderBufferBindingRanges::UBO_START + ShaderBufferBindingRanges::UBO_COUNT <=
                    ShaderBufferBindingRanges::SSBO_START,
                "UBO range overlaps with SSBO range");
  static_assert(ShaderBufferBindingRanges::SSBO_START + ShaderBufferBindingRanges::SSBO_COUNT <=
                    ShaderBufferBindingRanges::TEXTURE_START,
                "SSBO range overlaps with Texture range");
  static_assert(ShaderBufferBindingRanges::TEXTURE_START +
                        ShaderBufferBindingRanges::TEXTURE_COUNT <=
                    ShaderBufferBindingRanges::TOTAL_BINDING_POINTS,
                "Texture range exceeds total binding points");

  // 检查常用资源有足够的空间
  if (ShaderBufferBindingRanges::UBO_COUNT < 3) {
    LOG_WARN("UBO range may be too small for common resources");
  }
  if (ShaderBufferBindingRanges::SSBO_COUNT < 4) {
    LOG_WARN("SSBO range may be too small for common resources");
  }
  if (ShaderBufferBindingRanges::TEXTURE_COUNT < 32) {
    LOG_WARN("Texture range may be too small for complex scenes");
  }

  // 验证预定义的纹理绑定点在正确范围内
  static_assert(static_cast<uint32_t>(ShaderBufferResourceType::BaseColorTexture) >=
                    ShaderBufferBindingRanges::TEXTURE_START,
                "BaseColorTexture binding point out of range");
  static_assert(static_cast<uint32_t>(ShaderBufferResourceType::CustomTexture0) <
                    ShaderBufferBindingRanges::TEXTURE_START +
                        ShaderBufferBindingRanges::TEXTURE_COUNT,
                "CustomTexture0 binding point out of range");
  LOG_DEBUG("Binding point ranges validated successfully");
}
}  // namespace mite