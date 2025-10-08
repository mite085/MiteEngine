#include "material_template.h"

namespace mite {
MaterialTemplate::MaterialTemplate(){}

std::shared_ptr<MaterialInstance> MaterialTemplate::CreateInstance()
{
  auto instance = std::make_shared<MaterialInstance>();

  // 根据MaterialType和计数拼接默认创建的材质实例的名称
  std::string materialTypeName = GetMaterialTypeName();
  std::string numStr = std::to_string(m_DefaultInstanceCounter);

  // 补零到4位
  if (numStr.length() < 4) {
    materialTypeName.append(4 - numStr.length(), '0');
  }
  materialTypeName += numStr;

  // 设定名称，更新计数（TODO: 应当存在统一的管理器负责命名管理）
  instance->SetName(materialTypeName);
  m_DefaultInstanceCounter++;

  // 直接使用默认参数
  ApplyDefaultParams(instance);
  return instance;
}

std::shared_ptr<MaterialInstance> MaterialTemplate::CreateInstance(
    const MaterialSourceData &sourceData) const
{

  auto instance = std::make_shared<MaterialInstance>();
  // 设置材质名称
  if (!sourceData.name.empty()) {
    instance->SetName(sourceData.name);
  }
  // 初始化材质实例
  InitializeMaterialInstance(instance, sourceData);
  LOG_DEBUG("Created material instance '{}' of type '{}'", instance->GetName(), GetMaterialTypeName());
  return instance;
}

void MaterialTemplate::ApplyDefaultParams(std::shared_ptr<MaterialInstance> instance) const
{
  // 创建默认源数据并初始化实例
  MaterialSourceData defaultData = CreateDefaultSourceData();
  InitializeMaterialInstance(instance, defaultData);

  LOG_DEBUG("Applied default parameters to material instance '{}'", instance->GetName());
}

void MaterialTemplate::SetName(const std::string &name)
{
  m_Name = name;
}
const std::string &MaterialTemplate::GetName() const
{
  return m_Name;
}

const TextureGPUSlot *MaterialTemplate::GetTextureSlot(const MaterialSourceData &sourceData,
                                                       const std::string &slotName)
{
  auto it = sourceData.textureSlots.find(slotName);
  if (it != sourceData.textureSlots.end()) {
    return &it->second;
  }
  return nullptr;
}

bool MaterialTemplate::HasParameter(const MaterialSourceData &sourceData, const std::string &key)
{
  return sourceData.parameters.find(key) != sourceData.parameters.end();
}

bool MaterialTemplate::HasTextureSlot(const MaterialSourceData &sourceData,
                                      const std::string &slotName)
{
  return sourceData.textureSlots.find(slotName) != sourceData.textureSlots.end();
}

AlphaMode MaterialTemplate::GetAlphaMode(const MaterialSourceData &sourceData)
{
  return sourceData.alphaMode;
}
float MaterialTemplate::GetAlphaCutoff(const MaterialSourceData &sourceData)
{
  return sourceData.alphaCutoff;
}
bool MaterialTemplate::IsDoubleSided(const MaterialSourceData &sourceData)
{
  return sourceData.doubleSided;
}
void MaterialTemplate::FillMaterialDataFromSource(MaterialUniformBuffer &materialData,
                                                  const MaterialSourceData &sourceData)
{
  // 清空数据
  memset(&materialData, 0, sizeof(MaterialUniformBuffer));

  // ---- 材质类型 ----
  materialData.materialInfo = glm::vec4(
      static_cast<float>(static_cast<int>(sourceData.type)), 0.0f, 0.0f, 0.0f);

  // ---- 基础PBR参数 ----
  materialData.baseColor = GetParameter<glm::vec4>(
      sourceData, MaterialParamKeys::BASE_COLOR, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

  materialData.metallicRoughnessAO = glm::vec4(
      GetParameter<float>(sourceData, MaterialParamKeys::METALLIC, 0.0f),
      GetParameter<float>(sourceData, MaterialParamKeys::ROUGHNESS, 1.0f),
      GetParameter<float>(sourceData, MaterialParamKeys::AO, 1.0f),
      0.0f);

  // ---- 自发光合并Color和Intensity ---- 
  glm::vec3 emissionColor = GetParameter<glm::vec3>(
      sourceData, MaterialParamKeys::EMISSION_COLOR, glm::vec3(0.0f));
  float emissionIntensity = GetParameter<float>(
      sourceData, MaterialParamKeys::EMISSION_INTENSITY, 0.0f);
  materialData.emission = glm::vec4(emissionColor, emissionIntensity);

  // ---- 法线缩放 ---- 
  materialData.normalScale = glm::vec4(
      GetParameter<float>(sourceData, MaterialParamKeys::NORMAL_SCALE, 1.0f), 0.0f, 0.0f, 0.0f);

  // ---- 纹理标识 ----
  materialData.textureCNMROFlags = glm::vec4(
      HasTextureSlot(sourceData, MaterialParamKeys::BASE_COLOR_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::NORMAL_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE) ? 1.0f : 0.0f,
      HasTextureSlot(sourceData, MaterialParamKeys::OCCLUSION_TEXTURE) ? 1.0f : 0.0f);
  materialData.textureEmissionFlag = glm::vec4(
      HasTextureSlot(sourceData, MaterialParamKeys::EMISSIVE_TEXTURE) ? 1.0f : 0.0f,
      0.0f,
      0.0f,
      0.0f);

  // ---- 纹理参数 ----
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
  setupTexParams(MaterialParamKeys::BASE_COLOR_TEXTURE, materialData.baseColorTexParams);
  setupTexParams(MaterialParamKeys::NORMAL_TEXTURE, materialData.normalTexParams);
  setupTexParams(MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE, materialData.mrTexParams);
  setupTexParams(MaterialParamKeys::EMISSIVE_TEXTURE, materialData.emissiveTexParams);
  setupTexParams(MaterialParamKeys::OCCLUSION_TEXTURE, materialData.occlusionTexParams);

  // ---- 渲染属性 ----
  materialData.renderProperties = glm::vec4(
      sourceData.alphaCutoff,
      sourceData.doubleSided ? 1.0f : 0.0f,
      static_cast<float>(static_cast<int>(sourceData.alphaMode)),
      0.0f);
}
void MaterialTemplate::SetupMaterialTextures(std::shared_ptr<MaterialInstance> instance,
                                             const MaterialSourceData &sourceData)
{
  // 设置所有标准纹理槽位
  auto setupTexture = [&](const std::string &slotName) {
    if (HasTextureSlot(sourceData, slotName)) {
      const auto *slot = GetTextureSlot(sourceData, slotName);
      if (slot) {
        // 根据槽位名称设置对应的纹理
        if (slotName == MaterialParamKeys::BASE_COLOR_TEXTURE) {
          instance->SetBaseColorTexture(*slot);
        }
        else if (slotName == MaterialParamKeys::NORMAL_TEXTURE) {
          instance->SetNormalTexture(*slot);
        }
        else if (slotName == MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE) {
          instance->SetMetallicRoughnessTexture(*slot);
        }
        else if (slotName == MaterialParamKeys::EMISSIVE_TEXTURE) {
          instance->SetEmissiveTexture(*slot);
        }
        else if (slotName == MaterialParamKeys::OCCLUSION_TEXTURE) {
          instance->SetOcclusionTexture(*slot);
        }
        else {
          LOG_WARN("Unknown texture slot: {}", slotName);
        }
      }
    }
  };
  setupTexture(MaterialParamKeys::BASE_COLOR_TEXTURE);
  setupTexture(MaterialParamKeys::NORMAL_TEXTURE);
  setupTexture(MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE);
  setupTexture(MaterialParamKeys::EMISSIVE_TEXTURE);
  setupTexture(MaterialParamKeys::OCCLUSION_TEXTURE);
}
void MaterialTemplate::InitializeMaterialInstance(std::shared_ptr<MaterialInstance> instance,
                                                  const MaterialSourceData &sourceData) 
{
  // 初始化实例的UBO
  instance->InitializeUBO();
  // 从源数据填充材质数据
  MaterialUniformBuffer materialData;
  FillMaterialDataFromSource(materialData, sourceData);

  // 获取材质数据引用并更新
  MaterialUniformBuffer &instanceData = instance->GetMaterialData();
  instanceData = materialData;

  // 应用材质特定的纹理设置
  SetupMaterialTextures(instance, sourceData);
}
MaterialSourceData MaterialTemplate::CreateDefaultSourceData() const
{
  MaterialSourceData defaultData;
  defaultData.name = "Default_" + GetMaterialTypeName();

  // 设置默认参数
  defaultData.parameters[MaterialParamKeys::BASE_COLOR] = GetDefaultBaseColor();
  defaultData.parameters[MaterialParamKeys::METALLIC] = GetDefaultMetallic();
  defaultData.parameters[MaterialParamKeys::ROUGHNESS] = GetDefaultRoughness();
  defaultData.parameters[MaterialParamKeys::AO] = GetDefaultAO();
  defaultData.parameters[MaterialParamKeys::EMISSION_COLOR] = GetDefaultEmissionColor();
  defaultData.parameters[MaterialParamKeys::EMISSION_INTENSITY] = GetDefaultEmissionIntensity();
  defaultData.parameters[MaterialParamKeys::NORMAL_SCALE] = GetDefaultNormalScale();

  // 设置默认渲染属性
  defaultData.alphaMode = static_cast<AlphaMode>(GetDefaultAlphaMode());
  defaultData.alphaCutoff = GetDefaultAlphaCutoff();
  defaultData.doubleSided = GetDefaultDoubleSided();
  return defaultData;
}
};  // namespace mite