#include "gbuffer_material_template.h"
namespace mite {
GBufferMaterialTemplate::GBufferMaterialTemplate(std::shared_ptr<OpenGLShader> shader)
    : MaterialTemplate(std::move(shader)),
      m_BindingPoint(BindingPointManager::Get().GetMaterialUBOBinding())
{
  if (m_BindingPoint == UINT32_MAX) {
    LOG_ERROR("Failed to allocate binding point for GBufferMaterialTemplate");
  }
  else {
    LOG_DEBUG("GBufferMaterialTemplate allocated binding point: {}", m_BindingPoint);
  }
}
GBufferMaterialTemplate::~GBufferMaterialTemplate()
{
  if (m_BindingPoint != UINT32_MAX) {
    BindingPointManager::Get().ReleaseBindingPoint(m_BindingPoint);
    LOG_DEBUG("GBufferMaterialTemplate released binding point: {}", m_BindingPoint);
  }
}

std::shared_ptr<MaterialInstance> GBufferMaterialTemplate::CreateInstance(
    const MaterialSourceData &sourceData) const
{
  auto instance = std::make_shared<MaterialInstance>(m_Shader);

  // 设置材质名称
  if (!sourceData.name.empty()) {
    instance->SetName(sourceData.name);
  }
  // 设置纹理（纹理仍然需要单独绑定，因为它们是采样器）
  SetupTextures(instance, sourceData);

  // 为每个实例创建独立的UBO，确保数据隔离
  SetupInstanceUBO(instance, sourceData);

  LOG_DEBUG("Created material instance '{}' with independent UBO",
            sourceData.name.empty() ? "unnamed" : sourceData.name);

  return instance;
}
void GBufferMaterialTemplate::ApplyDefaultParams(std::shared_ptr<MaterialInstance> instance) const
{
  // 对于完全基于UBO的设计，ApplyDefaultParams主要用于创建默认实例
  // 实际参数通过UBO传递，这里只需要设置纹理标识等必要状态

  // 创建默认的SourceData来初始化UBO
  MaterialSourceData defaultData;
  defaultData.name = "DefaultMaterial";
  defaultData.parameters[MaterialParamKeys::BASE_COLOR] = GetDefaultBaseColor();
  defaultData.parameters[MaterialParamKeys::METALLIC] = GetDefaultMetallic();
  defaultData.parameters[MaterialParamKeys::ROUGHNESS] = GetDefaultRoughness();
  defaultData.parameters[MaterialParamKeys::AO] = GetDefaultAO();
  defaultData.parameters[MaterialParamKeys::EMISSION_COLOR] = GetDefaultEmissionColor();
  defaultData.parameters[MaterialParamKeys::EMISSION_INTENSITY] = GetDefaultEmissionIntensity();
  defaultData.parameters[MaterialParamKeys::NORMAL_SCALE] = GetDefaultNormalScale();

  // 为实例创建独立的默认UBO
  SetupInstanceUBO(instance, defaultData);
}
void GBufferMaterialTemplate::SetupInstanceUBO(std::shared_ptr<MaterialInstance> instance,
                                               const MaterialSourceData &sourceData) const
{
  // 初始化UBO数据
  MaterialUniformBuffer instanceUBOData = CreateUBOData(sourceData);
  // 为每个材质实例创建独立的UBO对象
  std::shared_ptr<ShaderUBO> instanceUBO = CreateInstanceUBO(instanceUBOData);

  // 将UBO绑定到材质实例，使用模板管理的绑定点
  // 注意：同一模板的不同实例共享绑定点，但拥有不同的UBO对象，维护不同的UBOData
  instance->SetupUBO(
      ShaderBufferResourceNames::MATERIAL_UBO, instanceUBO, instanceUBOData, m_BindingPoint);

  LOG_DEBUG("Setup independent UBO for material instance '{}' at binding point {}",
            instance->GetName(),
            m_BindingPoint);
}
std::shared_ptr<ShaderUBO> GBufferMaterialTemplate::CreateInstanceUBO(
    const MaterialUniformBuffer uniformdata) const
{
  // 创建新的UBO对象 (动态材质，需要每帧更新)
  std::shared_ptr<ShaderUBO> ubo = std::make_shared<ShaderUBO>(sizeof(MaterialUniformBuffer),
                                                               GL_DYNAMIC_DRAW);
  ubo->Initialize();

  // 首次更新UBO数据
  ubo->UpdateData(&uniformdata, sizeof(MaterialUniformBuffer));

  return ubo;
}

MaterialUniformBuffer GBufferMaterialTemplate::CreateUBOData(
    const MaterialSourceData &sourceData) const
{
  // 填充UBO数据
  MaterialUniformBuffer uboData;
  FillUBOData(uboData, sourceData);
  return uboData;

  LOG_DEBUG("Created independent UBO Data for material with {} parameters",
            sourceData.parameters.size());
}

// ---- 参数获取工具方法 ----
glm::vec4 GBufferMaterialTemplate::GetBaseColor(const MaterialSourceData &sourceData) const
{
  return GetParameter<glm::vec4>(sourceData, MaterialParamKeys::BASE_COLOR, GetDefaultBaseColor());
}
float GBufferMaterialTemplate::GetMetallic(const MaterialSourceData &sourceData) const
{
  return GetParameter<float>(sourceData, MaterialParamKeys::METALLIC, GetDefaultMetallic());
}
float GBufferMaterialTemplate::GetRoughness(const MaterialSourceData &sourceData) const
{
  return GetParameter<float>(sourceData, MaterialParamKeys::ROUGHNESS, GetDefaultRoughness());
}
float GBufferMaterialTemplate::GetAO(const MaterialSourceData &sourceData) const
{
  return GetParameter<float>(sourceData, MaterialParamKeys::AO, GetDefaultAO());
}
glm::vec3 GBufferMaterialTemplate::GetEmissionColor(const MaterialSourceData &sourceData) const
{
  return GetParameter<glm::vec3>(
      sourceData, MaterialParamKeys::EMISSION_COLOR, GetDefaultEmissionColor());
}
float GBufferMaterialTemplate::GetEmissionIntensity(const MaterialSourceData &sourceData) const
{
  return GetParameter<float>(
      sourceData, MaterialParamKeys::EMISSION_INTENSITY, GetDefaultEmissionIntensity());
}
float GBufferMaterialTemplate::GetNormalScale(const MaterialSourceData &sourceData) const
{
  return GetParameter<float>(sourceData, MaterialParamKeys::NORMAL_SCALE, GetDefaultNormalScale());
}
float GBufferMaterialTemplate::GetAlphaCutoff(const MaterialSourceData &sourceData) const
{
  return sourceData.alphaCutoff;
}
bool GBufferMaterialTemplate::GetDoubleSided(const MaterialSourceData &sourceData) const
{
  return sourceData.doubleSided;
}
float GBufferMaterialTemplate::GetAlphaMode(const MaterialSourceData &sourceData) const
{
  switch (sourceData.alphaMode) {
    case AlphaMode::OPAQUE:
      return 0.0f;
    case AlphaMode::MASK:
      return 1.0f;
    case AlphaMode::BLEND:
      return 2.0f;
    default:
      return 0.0f;
  }
}
// ---- 纹理处理工具方法 ----
void GBufferMaterialTemplate::SetupTextures(std::shared_ptr<MaterialInstance> instance,
                                            const MaterialSourceData &sourceData) const
{
  // 设置所有纹理槽位
  SetupTextureSlot(instance, MaterialParamKeys::BASE_COLOR_TEXTURE, sourceData);
  SetupTextureSlot(instance, MaterialParamKeys::NORMAL_TEXTURE, sourceData);
  SetupTextureSlot(instance, MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE, sourceData);
  SetupTextureSlot(instance, MaterialParamKeys::EMISSIVE_TEXTURE, sourceData);
  SetupTextureSlot(instance, MaterialParamKeys::OCCLUSION_TEXTURE, sourceData);
}
void GBufferMaterialTemplate::SetupTextureSlot(std::shared_ptr<MaterialInstance> instance,
                                               const std::string &slotName,
                                               const MaterialSourceData &sourceData) const
{
  if (HasTextureSlot(sourceData, slotName)) {
    const auto *slot = GetTextureSlot(sourceData, slotName);
    if (slot) {
      instance->SetTexture(slotName, *slot);
    }
  }
}
void GBufferMaterialTemplate::FillUBOData(MaterialUniformBuffer &uboData,
                                          const MaterialSourceData &sourceData) const
{
  // 清空UBO数据
  memset(&uboData, 0, sizeof(MaterialUniformBuffer));

  // ---- 基础PBR参数 ----
  uboData.baseColor = GetBaseColor(sourceData);
  uboData.metallicRoughnessAO = glm::vec4(
      GetMetallic(sourceData), GetRoughness(sourceData), GetAO(sourceData), 0.0f);

  // 自发光合并Color和Intensity
  glm::vec3 emissionColor = GetEmissionColor(sourceData);
  float emissionIntensity = GetEmissionIntensity(sourceData);
  uboData.emission = glm::vec4(emissionColor, emissionIntensity);

  // 法线缩放
  uboData.normalScale = glm::vec4(GetNormalScale(sourceData), 0.0f, 0.0f, 0.0f);

  // 纹理标识
  uboData.textureCNMROFlags = glm::vec4(
      HasTextureSlot(sourceData, MaterialParamKeys::BASE_COLOR_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::NORMAL_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::OCCLUSION_TEXTURE) ? 1.0f : 0.0f);

  uboData.textureEmissionFlag = glm::vec4(
      HasTextureSlot(sourceData, MaterialParamKeys::EMISSIVE_TEXTURE) ? 1.0f : 0.0f,
      0.0f,
      0.0f,
      0.0f);

  // 纹理参数（scale和offset）
  auto setupTexParams = [&](const std::string &slotName, glm::vec4 &params) {
    if (HasTextureSlot(sourceData, slotName)) {
      const auto *slot = GetTextureSlot(sourceData, slotName);
      if (slot) {
        params = glm::vec4(slot->scale, slot->offset);
        return;
      }
    }
    params = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);  // 默认参数
  };

  setupTexParams(MaterialParamKeys::BASE_COLOR_TEXTURE, uboData.baseColorTexParams);
  setupTexParams(MaterialParamKeys::NORMAL_TEXTURE, uboData.normalTexParams);
  setupTexParams(MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE, uboData.mrTexParams);
  setupTexParams(MaterialParamKeys::EMISSIVE_TEXTURE, uboData.emissiveTexParams);
  setupTexParams(MaterialParamKeys::OCCLUSION_TEXTURE, uboData.occlusionTexParams);

  // 渲染属性
  uboData.renderProperties = glm::vec4(GetAlphaCutoff(sourceData),
                                       GetDoubleSided(sourceData) ? 1.0f : 0.0f,
                                       GetAlphaMode(sourceData),
                                       0.0f);
}
}  // namespace mite