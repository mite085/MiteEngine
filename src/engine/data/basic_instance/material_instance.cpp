#include "material_instance.h"

namespace mite {
MaterialInstance::MaterialInstance(std::shared_ptr<OpenGLShader> shader)
    : m_Shader(std::move(shader))
{
  if (!m_Shader) {
    LOG_ERROR("MaterialInstance created with null shader!");
  }
}

MaterialInstance::~MaterialInstance() {}

// ===================== UBO管理 =====================
void MaterialInstance::InitializeUBO()
{
  if (m_UBO) {
    LOG_WARN("Material UBO already initialized");
    return;
  }
  // 创建并初始化UBO
  m_UBO = std::make_shared<ShaderUBO>(sizeof(MaterialUniformBuffer),
                                      ShaderBufferResourceType::MaterialUBO,
                                      "MaterialUBO_" + m_Name);

  m_UBO->Initialize();

  // 设置着色器绑定
  m_UBO->SetupShaderBinding(m_Shader, ShaderBufferResourceNames::MATERIAL_UBO);

  // 上传初始数据
  UpdateUBO();

  LOG_INFO("Material UBO initialized for '{}'", m_Name);
}

void MaterialInstance::UpdateUBO()
{
  if (!m_UBO || !m_UBO->IsInitialized()) {
    LOG_ERROR("Cannot update UBO: not initialized");
    return;
  }

  m_UBO->UpdateData(&m_MaterialData, sizeof(MaterialUniformBuffer));
}

// ===================== MaterialUniformBuffer 设置接口 =====================
void MaterialInstance::SetMaterialInfo(MaterialType type)
{
  m_MaterialData.materialInfo.x = static_cast<float>(static_cast<int>(type));
  m_MaterialData.materialInfo.y = 0.0f;
  m_MaterialData.materialInfo.z = 0.0f;
  m_MaterialData.materialInfo.w = 0.0f;
}
// ---- 基础PBR参数 ----
void MaterialInstance::SetBaseColor(const glm::vec4 &color)
{
  m_MaterialData.baseColor = color;
}
void MaterialInstance::SetMetallic(float metallic)
{
  m_MaterialData.metallicRoughnessAO.x = metallic;
}
void MaterialInstance::SetRoughness(float roughness)
{
  m_MaterialData.metallicRoughnessAO.y = roughness;
}
void MaterialInstance::SetAO(float ao)
{
  m_MaterialData.metallicRoughnessAO.z = ao;
}
void MaterialInstance::SetEmission(const glm::vec4 &emission)
{
  m_MaterialData.emission = emission;
}
void MaterialInstance::SetNormalScale(float scale)
{
  m_MaterialData.normalScale.x = scale;
}
// ---- NPR参数 ----
void MaterialInstance::SetNPRParameters(const glm::vec4 &params)
{
  m_MaterialData.nprParameters = params;
}
void MaterialInstance::SetNPRColors(const glm::vec4 &colors)
{
  m_MaterialData.nprColors = colors;
}
void MaterialInstance::SetRampThreshold(float threshold)
{
  m_MaterialData.nprParameters.x = threshold;
}
void MaterialInstance::SetRampSmoothness(float smoothness)
{
  m_MaterialData.nprParameters.y = smoothness;
}
void MaterialInstance::SetSpecularSize(float size)
{
  m_MaterialData.nprParameters.z = size;
}
void MaterialInstance::SetOutlineWidth(float width)
{
  m_MaterialData.nprParameters.w = width;
}
void MaterialInstance::SetShadowTint(const glm::vec3 &tint)
{
  m_MaterialData.nprColors.x = tint.x;
  m_MaterialData.nprColors.y = tint.y;
  m_MaterialData.nprColors.z = tint.z;
}
void MaterialInstance::SetRimPower(float power)
{
  m_MaterialData.nprColors.w = power;
}
// ---- 纹理标志设置 ----
void MaterialInstance::SetBaseColorTextureEnabled(bool enabled)
{
  m_MaterialData.textureCNMROFlags.x = enabled ? 1.0f : 0.0f;
}
void MaterialInstance::SetNormalTextureEnabled(bool enabled)
{
  m_MaterialData.textureCNMROFlags.y = enabled ? 1.0f : 0.0f;
}
void MaterialInstance::SetMetallicRoughnessTextureEnabled(bool enabled)
{
  m_MaterialData.textureCNMROFlags.z = enabled ? 1.0f : 0.0f;
}
void MaterialInstance::SetOcclusionTextureEnabled(bool enabled)
{
  m_MaterialData.textureCNMROFlags.w = enabled ? 1.0f : 0.0f;
}
void MaterialInstance::SetEmissiveTextureEnabled(bool enabled)
{
  m_MaterialData.textureEmissionFlag.x = enabled ? 1.0f : 0.0f;
}
// ---- 纹理参数设置 ----
void MaterialInstance::SetBaseColorTexParams(const glm::vec4 &params)
{
  m_MaterialData.baseColorTexParams = params;
}
void MaterialInstance::SetNormalTexParams(const glm::vec4 &params)
{
  m_MaterialData.normalTexParams = params;
}
void MaterialInstance::SetMRTexParams(const glm::vec4 &params)
{
  m_MaterialData.mrTexParams = params;
}
void MaterialInstance::SetEmissiveTexParams(const glm::vec4 &params)
{
  m_MaterialData.emissiveTexParams = params;
}
void MaterialInstance::SetOcclusionTexParams(const glm::vec4 &params)
{
  m_MaterialData.occlusionTexParams = params;
}
// ---- 渲染属性设置 ----
void MaterialInstance::SetAlphaCutoff(float cutoff)
{
  m_MaterialData.renderProperties.x = cutoff;
}
void MaterialInstance::SetDoubleSided(bool doubleSided)
{
  m_MaterialData.renderProperties.y = doubleSided ? 1.0f : 0.0f;
}
void MaterialInstance::SetAlphaMode(int mode)
{
  m_MaterialData.renderProperties.z = static_cast<float>(mode);
}
// ===================== 纹理绑定 =====================
void MaterialInstance::SetupTextureBinding(TextureGPUSlot texture,
                                           uint32_t bindingPoint,
                                           const std::string &samplerName)
{
  // 移除已存在的相同绑定点纹理
  m_Textures.erase(std::remove_if(m_Textures.begin(),
                                  m_Textures.end(),
                                  [bindingPoint](const TextureSlot &slot) {
                                    return slot.bindingPoint == bindingPoint;
                                  }),
                   m_Textures.end());

  // 添加新纹理
  m_Textures.push_back({texture, bindingPoint, samplerName});
}
void MaterialInstance::SetBaseColorTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(
      texture, TextureBindingPoints::BASE_COLOR, ShaderBufferResourceNames::BASE_COLOR_TEXTURE);
  SetBaseColorTextureEnabled(true);
}
void MaterialInstance::SetNormalTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(
      texture, TextureBindingPoints::NORMAL, ShaderBufferResourceNames::NORMAL_TEXTURE);
  SetNormalTextureEnabled(true);
}
void MaterialInstance::SetMetallicRoughnessTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(texture,
                      TextureBindingPoints::METALLIC_ROUGHNESS,
                      ShaderBufferResourceNames::METALLIC_ROUGHNESS_TEXTURE);
  SetMetallicRoughnessTextureEnabled(true);
}
void MaterialInstance::SetEmissiveTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(
      texture, TextureBindingPoints::EMISSIVE, ShaderBufferResourceNames::EMISSIVE_TEXTURE);
  SetEmissiveTextureEnabled(true);
}
void MaterialInstance::SetOcclusionTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(
      texture, TextureBindingPoints::OCCLUSION, ShaderBufferResourceNames::OCCLUSION_TEXTURE);
  SetOcclusionTextureEnabled(true);
}
// ===================== 绑定相关 =====================
void MaterialInstance::BindShaderOnly(OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader to bind!");
    return;
  }
  targetShader->Bind();
}
size_t MaterialInstance::BindTexturesOnly(TextureBindFunc textureBindFunc,
                                          OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader for texture binding!");
    return 0;
  }
  // 绑定纹理到预定义的绑定点
  for (const auto &textureSlot : m_Textures) {
    // 使用传入的纹理绑定函数进行纹理绑定
    textureBindFunc(textureSlot.texture.gpuHandle, textureSlot.bindingPoint);

    // 设置着色器采样器绑定点
    targetShader->SetTextureBinding(textureSlot.samplerName, textureSlot.bindingPoint);

    // TODO: 需要将Solt的offset和scale传入Shader
    // textureSlot.texture.offset;
    // textureSlot.texture.scale;
  }
  return m_Textures.size();
}

void MaterialInstance::BindBuffersOnly() const
{
  if (m_UBO && m_UBO->IsInitialized()) {
    // 绑定UBO（在绑定Shader后立即执行）
    m_UBO->Bind();
  }
}

void MaterialInstance::Apply(TextureBindFunc textureBindFunc, OpenGLShader *overrideShader) const
{
  BindShaderOnly(overrideShader);
  BindBuffersOnly();
  BindTexturesOnly(textureBindFunc, overrideShader);
}

std::shared_ptr<OpenGLShader> MaterialInstance::GetShader() const
{
  if (m_Shader)
    return m_Shader;
  else {
    LOG_ERROR("Invaid Shader");
    return nullptr;
  }
}
std::string MaterialInstance::GetName() const
{
  return m_Name;
}
void MaterialInstance::SetName(const std::string &name)
{
  m_Name = name;
}
};  // namespace mite