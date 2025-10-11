#include "shader_binding_point_manager.h"

namespace mite {
BindingPointManager &BindingPointManager::Get()
{
  static BindingPointManager instance;
  return instance;
}

BindingPointManager::BindingPointManager()
{
  // 初始化 UBO 下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(UBOResourceType::Count); ++i) {
    m_NextUBOPoints[i] = 0;  // UBO 绑定点从 0 开始
  }
  // 初始化 SSBO 下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(SSBOResourceType::Count); ++i) {
    m_NextSSBOPoints[i] = 0;  // SSBO 绑定点从 0 开始
  }
  // 初始化纹理下一个绑定点
  for (size_t i = 0; i < static_cast<size_t>(TextureResourceType::Count); ++i) {
    m_NextTexturePoints[i] = 0;  // 纹理单元从 0 开始
  }
  LOG_INFO("BindingPointManager initialized with separate namespaces:");
  LOG_INFO("  Texture Units: {}", BindingRanges::GetMaxTextureUnits());
  LOG_INFO("  UBO Bindings: {}", BindingRanges::GetMaxUBOBindings());
  LOG_INFO("  SSBO Bindings: {}", BindingRanges::GetMaxSSBOBindings());
}

// ---- UBO 绑定点管理 ----
uint32_t BindingPointManager::AllocateUBOBinding(UBOResourceType type, const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  size_t typeIndex = static_cast<size_t>(type);
  return AllocateFromRange(
      m_AllocatedUBOs, 0, BindingRanges::GetMaxUBOBindings(), m_NextUBOPoints[typeIndex], name);
}
void BindingPointManager::ReleaseUBOBinding(uint32_t bindingPoint)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (bindingPoint < BindingRanges::GetMaxUBOBindings()) {
    ReleaseFromRange(m_AllocatedUBOs, bindingPoint);
  }
}
bool BindingPointManager::IsUBOBindingAllocated(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return bindingPoint < BindingRanges::GetMaxUBOBindings() && m_AllocatedUBOs.test(bindingPoint);
}
// ---- SSBO 绑定点管理 ----
uint32_t BindingPointManager::AllocateSSBOBinding(SSBOResourceType type, const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  size_t typeIndex = static_cast<size_t>(type);
  return AllocateFromRange(
      m_AllocatedSSBOs, 0, BindingRanges::GetMaxSSBOBindings(), m_NextSSBOPoints[typeIndex], name);
}
void BindingPointManager::ReleaseSSBOBinding(uint32_t bindingPoint)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (bindingPoint < BindingRanges::GetMaxSSBOBindings()) {
    ReleaseFromRange(m_AllocatedSSBOs, bindingPoint);
  }
}
bool BindingPointManager::IsSSBOBindingAllocated(uint32_t bindingPoint) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return bindingPoint < BindingRanges::GetMaxSSBOBindings() && m_AllocatedSSBOs.test(bindingPoint);
}
// ---- 纹理绑定点管理 ----
uint32_t BindingPointManager::AllocateTextureBinding(TextureResourceType category,
                                                     const std::string &name)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  size_t typeIndex = static_cast<size_t>(category);
  return AllocateFromRange(m_AllocatedTextures,
                           0,
                           BindingRanges::GetMaxTextureUnits(),
                           m_NextTexturePoints[typeIndex],
                           name);
}
void BindingPointManager::ReleaseTextureBinding(uint32_t textureUnit)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (textureUnit < BindingRanges::GetMaxTextureUnits()) {
    ReleaseFromRange(m_AllocatedTextures, textureUnit);
  }
}
bool BindingPointManager::IsTextureBindingAllocated(uint32_t textureUnit) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return textureUnit < BindingRanges::GetMaxTextureUnits() &&
         m_AllocatedTextures.test(textureUnit);
}

// ---- 纹理类型映射接口 ----
uint32_t BindingPointManager::GetRuntimeTextureBinding(RuntimeTextureType type) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_RuntimeTextureBindings.find(type);
  return it != m_RuntimeTextureBindings.end() ? it->second : UINT32_MAX;
}
uint32_t BindingPointManager::GetExternalTextureBinding(ExternalTextureType type) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_ExternalTextureBindings.find(type);
  return it != m_ExternalTextureBindings.end() ? it->second : UINT32_MAX;
}

void BindingPointManager::Reset()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 重置所有分配状态
  m_AllocatedUBOs.reset();
  m_AllocatedSSBOs.reset();
  m_AllocatedTextures.reset();

  m_RuntimeTextureBindings.clear();
  m_ExternalTextureBindings.clear();

  // 重置预分配资源
  m_CameraUBOBinding = UINT32_MAX;
  m_MaterialUBOBinding = UINT32_MAX;
  m_ModelUBOBinding = UINT32_MAX;
  m_LightSSBOBinding = UINT32_MAX;

  // 重置下一个绑定点
  for (auto &point : m_NextUBOPoints)
    point = 0;
  for (auto &point : m_NextSSBOPoints)
    point = 0;
  for (auto &point : m_NextTexturePoints)
    point = 0;

  LOG_INFO("BindingPointManager reset");
}

// ---- 内部分配方法 ----
uint32_t BindingPointManager::AllocateFromRange(std::bitset<1024> &allocated,
                                                uint32_t rangeStart,
                                                uint32_t rangeCount,
                                                std::atomic<uint32_t> &nextPoint,
                                                const std::string &name)
{
  uint32_t start = nextPoint.load();
  uint32_t current = start;
  uint32_t rangeEnd = rangeStart + rangeCount;
  do {
    // 先执行查询，查询到空位则增加新的绑定点
    if (current < rangeEnd && !allocated.test(current)) {
      allocated.set(current);
      nextPoint = (current + 1) % rangeEnd;
      if (nextPoint < rangeStart)
        nextPoint = rangeStart;

      LOG_DEBUG("Allocated binding point {} for {}", current, name);
      return current;
    }

    // 查询下一位
    current = (current + 1) % rangeEnd;
    if (current < rangeStart)
      current = rangeStart; // 复位，退出循环
  } while (current != start);
  LOG_ERROR("Failed to allocate binding point for {}: no available points", name);
  return UINT32_MAX;
}
void BindingPointManager::ReleaseFromRange(std::bitset<1024> &allocated, uint32_t point)
{
  if (allocated.test(point)) {
    allocated.reset(point);
    LOG_DEBUG("Released binding point: {}", point);
  }
}

void BindingPointManager::PreallocateCommonResources()
{
  // UBO 预分配
  m_CameraUBOBinding = AllocateUBOBinding(UBOResourceType::CameraUBO,
                                          ShaderBufferResourceNames::CAMERA_UBO);
  m_MaterialUBOBinding = AllocateUBOBinding(UBOResourceType::MaterialUBO,
                                            ShaderBufferResourceNames::MATERIAL_UBO);
  m_ModelUBOBinding = AllocateUBOBinding(UBOResourceType::ModelUBO,
                                         ShaderBufferResourceNames::MODEL_UBO);
  // SSBO 预分配
  m_LightSSBOBinding = AllocateSSBOBinding(SSBOResourceType::LightSSBO,
                                           ShaderBufferResourceNames::LIGHT_SSBO);
  // 预分配运行时纹理资源并建立映射
  // GBuffer_WorldPosDepth----layout(location = 0)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_WorldPosDepth] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture,
      ShaderBufferResourceNames::GBUFFER_WORLD_POS_DEPTH);
  // GBuffer_BaseColorMatType----layout(location = 1)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_BaseColorMatType] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture,
      ShaderBufferResourceNames::GBUFFER_BASE_COLOR_MAT_TYPE);
  // GBuffer_MetallicRoughnessAO----layout(location = 2)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_MetallicRoughnessAO] =
      AllocateTextureBinding(TextureResourceType::RuntimeTexture,
                             ShaderBufferResourceNames::GBUFFER_METALLIC_ROUGHNESS_AO);
  // GBuffer_NormalScale----layout(location = 3)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_NormalScale] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::GBUFFER_NORMAL_SCALE);
  // GBuffer_EmissionAlpha----layout(location = 4)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_EmissionAlpha] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::GBUFFER_EMISSION_ALPHA);
  // GBuffer_NPRParam----layout(location = 5)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_NPRParam] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::GBUFFER_NPR_PARAM);
  // GBuffer_NPRColor----layout(location = 6)
  m_RuntimeTextureBindings[RuntimeTextureType::GBuffer_NPRColor] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::GBUFFER_NPR_COLOR);
  // ShadowMap_Directional----layout(location = 7)
  m_RuntimeTextureBindings[RuntimeTextureType::ShadowMap_Directional] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::SHADOW_MAP_DIRECTIONAL);
  // ShadowMap_Point----layout(location = 8)
  m_RuntimeTextureBindings[RuntimeTextureType::ShadowMap_Point] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::SHADOW_MAP_POINT);
  // ShadowMap_Spot----layout(location = 9)
  m_RuntimeTextureBindings[RuntimeTextureType::ShadowMap_Spot] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::SHADOW_MAP_SPOT);
  // ShadowMap_Area----layout(location = 10)
  m_RuntimeTextureBindings[RuntimeTextureType::ShadowMap_Area] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::SHADOW_MAP_AREA);
  // Lighting_Diffuse----layout(location = 11)
  m_RuntimeTextureBindings[RuntimeTextureType::Lighting_Diffuse] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::LIGHTING_DIFFUSE);
  // Lighting_Specular----layout(location = 12)
  m_RuntimeTextureBindings[RuntimeTextureType::Lighting_Specular] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::LIGHTING_SPECULAR);
  // Lighting_Combined----layout(location = 13)
  m_RuntimeTextureBindings[RuntimeTextureType::Lighting_Combined] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::LIGHTING_COMBINED);
  // Lighting_Ambient----layout(location = 14)
  m_RuntimeTextureBindings[RuntimeTextureType::Lighting_Ambient] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::LIGHTING_AMBIENT);

  //m_RuntimeTextureBindings[RuntimeTextureType::PostProcess_Bloom] = AllocateTextureBinding(
  //    TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::POSTPROCESS_BLOOM);
  //m_RuntimeTextureBindings[RuntimeTextureType::PostProcess_ToneMapped] = AllocateTextureBinding(
  //    TextureResourceType::RuntimeTexture,
  //    ShaderBufferResourceNames::POSTPROCESS_TONE_MAPPED);
  //m_RuntimeTextureBindings[RuntimeTextureType::PostProcess_Final] = AllocateTextureBinding(
  //    TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::POSTPROCESS_FINAL);

  //  RenderTarget----layout(location = 15)
  m_RuntimeTextureBindings[RuntimeTextureType::RenderTarget] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::RENDER_TARGET);
  // Depth----layout(location = 16)
  m_RuntimeTextureBindings[RuntimeTextureType::Depth] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::DEPTH_TEXTURE);
  // Stencil----layout(location = 17)
  m_RuntimeTextureBindings[RuntimeTextureType::Stencil] = AllocateTextureBinding(
      TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::STENCIL_TEXTURE);

  //m_RuntimeTextureBindings[RuntimeTextureType::Debug_View] = AllocateTextureBinding(
  //    TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::DEBUG_VIEW);
  //m_RuntimeTextureBindings[RuntimeTextureType::UI_Overlay] = AllocateTextureBinding(
  //    TextureResourceType::RuntimeTexture, ShaderBufferResourceNames::UI_OVERLAY);
  
  // 预分配外部加载纹理资源并建立映射
  // BaseColor----layout(location = 18)
  m_ExternalTextureBindings[ExternalTextureType::BaseColor] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture, ShaderBufferResourceNames::BASE_COLOR_TEXTURE);
  // Normal----layout(location = 19)
  m_ExternalTextureBindings[ExternalTextureType::Normal] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture, ShaderBufferResourceNames::NORMAL_TEXTURE);
  // MetallicRoughness----layout(location = 20)
  m_ExternalTextureBindings[ExternalTextureType::MetallicRoughness] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture,
      ShaderBufferResourceNames::METALLIC_ROUGHNESS_TEXTURE);
  // Emissive----layout(location = 21)
  m_ExternalTextureBindings[ExternalTextureType::Emissive] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture, ShaderBufferResourceNames::EMISSIVE_TEXTURE);
  // Occlusion----layout(location = 22)
  m_ExternalTextureBindings[ExternalTextureType::Occlusion] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture, ShaderBufferResourceNames::OCCLUSION_TEXTURE);
  // EnvironmentMap----layout(location = 23)
  m_ExternalTextureBindings[ExternalTextureType::EnvironmentMap] = AllocateTextureBinding(
      TextureResourceType::ExternalTexture, ShaderBufferResourceNames::ENVIRONMENT_MAP);

  //m_ExternalTextureBindings[ExternalTextureType::BRDFLUT] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::BRDF_LUT);
  //m_ExternalTextureBindings[ExternalTextureType::IrradianceMap] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::IRRADIANCE_MAP);
  //m_ExternalTextureBindings[ExternalTextureType::PrefilterMap] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::PREFILTER_MAP);
  //m_ExternalTextureBindings[ExternalTextureType::ColorGradingLUT] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::COLOR_GRADING_LUT);
  //m_ExternalTextureBindings[ExternalTextureType::BloomTexture] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::BLOOM_TEXTURE);
  //m_ExternalTextureBindings[ExternalTextureType::SSAOTexture] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::SSAO_TEXTURE);
  //m_ExternalTextureBindings[ExternalTextureType::Custom0] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::CUSTOM_TEXTURE_0);
  //m_ExternalTextureBindings[ExternalTextureType::Custom1] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::CUSTOM_TEXTURE_1);
  //m_ExternalTextureBindings[ExternalTextureType::Custom2] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::CUSTOM_TEXTURE_2);
  //m_ExternalTextureBindings[ExternalTextureType::Custom3] = AllocateTextureBinding(
  //    TextureResourceType::ExternalTexture, ShaderBufferResourceNames::CUSTOM_TEXTURE_3);
  LOG_INFO("Preallocated common resources:");
  LOG_INFO("  UBOs: Camera={}, Material={}, Model={}",
           m_CameraUBOBinding,
           m_MaterialUBOBinding,
           m_ModelUBOBinding);
  LOG_INFO("  SSBOs: Lights={}", m_LightSSBOBinding);
  LOG_INFO("  Textures: Runtime={} types, External={} types",
           m_RuntimeTextureBindings.size(),
           m_ExternalTextureBindings.size());
}

}  // namespace mite