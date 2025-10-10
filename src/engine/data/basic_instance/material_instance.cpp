#include "material_instance.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
// ===================== UBO管理 =====================
void MaterialInstance::InitializeUBO()
{
  if (m_UBO) {
    LOG_WARN("Material UBO already initialized");
    return;
  }
  // 创建并初始化UBO
  m_UBO = std::make_shared<ShaderUBO>(sizeof(MaterialUniformBuffer),
                                      BindingPointManager::Get().GetMaterialUBOBinding(),
                                      GL_DYNAMIC_DRAW);
  m_UBO->Initialize();

  LOG_INFO("Material UBO initialized for '{}'", m_Name);
}

void MaterialInstance::SetupShaderBinding(std::shared_ptr<OpenGLShader> shader)
{
  // 设置着色器绑定
  m_UBO->SetupShaderBinding(shader, ShaderBufferResourceNames::MATERIAL_UBO);
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
MaterialType MaterialInstance::GetMaterialType() const
{
  return static_cast<MaterialType>(static_cast<int>(m_MaterialData.materialInfo.x));
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
glm::vec3 MaterialInstance::GetShadowTint() const
{
  return glm::vec3(
      m_MaterialData.nprColors.x, m_MaterialData.nprColors.y, m_MaterialData.nprColors.z);
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
void MaterialInstance::SetupTextureBinding(TextureGPUSlot texture, ExternalTextureType type)
{
  // 通过类型查询绑定点管理器获取绑定点
  uint32_t bindingPoint = BindingPointManager::Get().GetExternalTextureBinding(type);

  // 移除已存在的相同绑定点纹理
  if (m_Textures.find(type) != m_Textures.end())
    m_Textures.erase(type);

  // 添加新纹理
  m_Textures.insert({type, texture});
}
void MaterialInstance::SetBaseColorTexture(TextureGPUSlot texture)
{
  // 设定纹理绑定
  SetupTextureBinding(texture, ExternalTextureType::BaseColor);

  // UBO更新纹理偏移/缩放
  SetBaseColorTexParams(glm::vec4{texture.scale, texture.offset});

  // 启用纹理
  SetBaseColorTextureEnabled(true);
}
void MaterialInstance::SetNormalTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(texture, ExternalTextureType::Normal);
  SetNormalTexParams(glm::vec4{texture.scale, texture.offset});
  SetNormalTextureEnabled(true);
}
void MaterialInstance::SetMetallicRoughnessTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(texture, ExternalTextureType::MetallicRoughness);
  SetMRTexParams(glm::vec4{texture.scale, texture.offset});
  SetMetallicRoughnessTextureEnabled(true);
}
void MaterialInstance::SetEmissiveTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(texture, ExternalTextureType::Emissive);
  SetEmissiveTexParams(glm::vec4{texture.scale, texture.offset});
  SetEmissiveTextureEnabled(true);
}
void MaterialInstance::SetOcclusionTexture(TextureGPUSlot texture)
{
  SetupTextureBinding(texture, ExternalTextureType::Occlusion);
  SetOcclusionTexParams(glm::vec4{texture.scale, texture.offset});
  SetOcclusionTextureEnabled(true);
}
// ===================== 绑定相关 =====================
size_t MaterialInstance::BindTexturesOnly(ExternalTextureBindFunc textureBindFunc) const
{
  // 绑定纹理到预定义的绑定点
  for (const auto [textureType, textureSlot] : m_Textures) {
    // 使用传入的纹理绑定函数进行纹理绑定
    textureBindFunc(textureType, textureSlot.gpuHandle, textureSlot.target);

    // 注意：Texture的offset和scale通过MaterialUBO
    // 的m_MaterialData.baseColorTexParams等参数完成传递
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

void MaterialInstance::Apply(ExternalTextureBindFunc textureBindFunc) const
{
  BindBuffersOnly();
  BindTexturesOnly(textureBindFunc);
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