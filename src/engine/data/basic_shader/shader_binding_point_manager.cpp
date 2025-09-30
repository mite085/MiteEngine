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
  for (size_t i = 0; i < static_cast<size_t>(ResourceType::Count); ++i) {
    ResourceType type = static_cast<ResourceType>(i);
    m_NextBindingPoints[i] = GetRangeStart(type);
  }

  // 验证范围设置
  ValidateRanges();

  LOG_INFO("BindingPointManager initialized with {} total binding points",
           BindingRanges::TOTAL_BINDING_POINTS);
}

uint32_t BindingPointManager::AllocateBindingPoint(ResourceType type, const std::string &name)
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
      m_ResourceNames[currentPoint] = name.empty() ? "Unnamed_" + std::to_string(currentPoint) :
                                                     name;

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
  m_ResourceNames.erase(bindingPoint);
  m_ResourceTypes.erase(bindingPoint);

  LOG_DEBUG("Released binding point: {}", bindingPoint);
}

void BindingPointManager::PreallocateCommonResources()
{
  // 预分配相机UBO
  m_CameraUBOBinding = AllocateBindingPoint(ResourceType::CameraUBO, "CameraUBO");

  // 预分配光源SSBO
  m_LightSSBOBinding = AllocateBindingPoint(ResourceType::LightSSBO, "LightSSBO");

  // 预分配阴影贴图
  m_ShadowMapBinding = AllocateBindingPoint(ResourceType::ShadowMap, "ShadowMap");

  LOG_INFO("Preallocated common resources: CameraUBO={}, LightSSBO={}, ShadowMap={}",
           m_CameraUBOBinding,
           m_LightSSBOBinding,
           m_ShadowMapBinding);
}

std::string BindingPointManager::GetResourceName(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_ResourceNames.find(bindingPoint);
  if (it != m_ResourceNames.end()) {
    return it->second;
  }
  return "";
}

BindingPointManager::ResourceType BindingPointManager::GetResourceType(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_ResourceTypes.find(bindingPoint);
  if (it != m_ResourceTypes.end()) {
    return it->second;
  }
  return ResourceType::Count;  // 无效类型
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
  m_ResourceNames.clear();
  m_ResourceTypes.clear();

  // 重置各类型的下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(ResourceType::Count); ++i) {
    ResourceType type = static_cast<ResourceType>(i);
    m_NextBindingPoints[i] = GetRangeStart(type);
  }

  LOG_INFO("BindingPointManager reset");
}

// ---- 内部方法 ----

uint32_t BindingPointManager::GetRangeStart(ResourceType type) const
{
  switch (type) {
    case ResourceType::CameraUBO:
    case ResourceType::MaterialUBO:
    case ResourceType::SceneUBO:
    case ResourceType::CustomUBO:
      return BindingRanges::UBO_START;

    case ResourceType::LightSSBO:
    case ResourceType::InstanceSSBO:
    case ResourceType::BoneSSBO:
    case ResourceType::ComputeSSBO:
    case ResourceType::CustomSSBO:
      return BindingRanges::SSBO_START;

    case ResourceType::ShadowMap:
    case ResourceType::EnvironmentMap:
    case ResourceType::BRDFLUT:
      return BindingRanges::TEXTURE_START;

    default:
      return 0;
  }
}

uint32_t BindingPointManager::GetRangeCount(ResourceType type) const
{
  switch (type) {
    case ResourceType::CameraUBO:
    case ResourceType::MaterialUBO:
    case ResourceType::SceneUBO:
    case ResourceType::CustomUBO:
      return BindingRanges::UBO_COUNT;

    case ResourceType::LightSSBO:
    case ResourceType::InstanceSSBO:
    case ResourceType::BoneSSBO:
    case ResourceType::ComputeSSBO:
    case ResourceType::CustomSSBO:
      return BindingRanges::SSBO_COUNT;

    case ResourceType::ShadowMap:
    case ResourceType::EnvironmentMap:
    case ResourceType::BRDFLUT:
      return BindingRanges::TEXTURE_COUNT;

    default:
      return 0;
  }
}

bool BindingPointManager::IsValidBindingPoint(uint32_t point) const
{
  return point < BindingRanges::TOTAL_BINDING_POINTS;
}

void BindingPointManager::ValidateRanges() const
{
  // 检查范围不重叠
  static_assert(BindingRanges::UBO_START + BindingRanges::UBO_COUNT <= BindingRanges::SSBO_START,
                "UBO range overlaps with SSBO range");
  static_assert(BindingRanges::SSBO_START + BindingRanges::SSBO_COUNT <=
                    BindingRanges::TEXTURE_START,
                "SSBO range overlaps with Texture range");
  static_assert(BindingRanges::TEXTURE_START + BindingRanges::TEXTURE_COUNT <=
                    BindingRanges::TOTAL_BINDING_POINTS,
                "Texture range exceeds total binding points");

  // 检查常用资源有足够的空间
  if (BindingRanges::UBO_COUNT < 3) {
    LOG_WARN("UBO range may be too small for common resources");
  }
  if (BindingRanges::SSBO_COUNT < 4) {
    LOG_WARN("SSBO range may be too small for common resources");
  }
  if (BindingRanges::TEXTURE_COUNT < 32) {
    LOG_WARN("Texture range may be too small for complex scenes");
  }
}

}  // namespace mite
