#include "material_template_gltf_pbr.h"

namespace mite {
GLTFPBRMaterialTemplate::GLTFPBRMaterialTemplate(std::shared_ptr<OpenGLShader> shader)
    : MaterialTemplate(shader)
{
}

std::shared_ptr<MaterialInstance> GLTFPBRMaterialTemplate::CreateInstance(
    const MaterialSourceData &sourceData) const
{
  auto instance = std::make_shared<MaterialInstance>(m_Shader);

  // 设置实例名称
  instance->SetName(sourceData.name);

  // 应用源数据中的所有参数
  ApplyPBRParameters(*instance, sourceData);
  ApplyTextureSlots(*instance, sourceData);
  ApplyRenderProperties(*instance, sourceData);

  return instance;
}

void GLTFPBRMaterialTemplate::ApplyDefaultParams(MaterialInstance &instance) const
{
  // 设置默认PBR参数
  instance.SetVector4("u_BaseColorFactor", m_DefaultBaseColor);
  instance.SetFloat("u_MetallicFactor", m_DefaultMetallic);
  instance.SetFloat("u_RoughnessFactor", m_DefaultRoughness);
  instance.SetVector3("u_EmissiveFactor", m_DefaultEmissive);

  // 设置默认渲染属性
  SetupAlphaBlending(instance, m_DefaultAlphaMode, m_DefaultAlphaCutoff);
  instance.SetInt("u_DoubleSided", m_DefaultDoubleSided ? 1 : 0);
}

void GLTFPBRMaterialTemplate::ApplyPBRParameters(MaterialInstance &instance,
                                                 const MaterialSourceData &sourceData) const
{
  // 使用基类工具方法获取参数（带默认值）
  glm::vec4 baseColor = GetParameter(sourceData, "baseColorFactor", m_DefaultBaseColor);
  float metallic = GetParameter(sourceData, "metallicFactor", m_DefaultMetallic);
  float roughness = GetParameter(sourceData, "roughnessFactor", m_DefaultRoughness);
  glm::vec3 emissive = GetParameter(sourceData, "emissiveFactor", m_DefaultEmissive);

  // 设置PBR参数到着色器
  instance.SetVector4("u_BaseColorFactor", baseColor);
  instance.SetFloat("u_MetallicFactor", metallic);
  instance.SetFloat("u_RoughnessFactor", roughness);
  instance.SetVector3("u_EmissiveFactor", emissive);

  // 设置纹理存在标志（告诉着色器哪些纹理可用）
  instance.SetInt("u_HasBaseColorTexture", HasTextureSlot(sourceData, "baseColorTexture") ? 1 : 0);
  instance.SetInt("u_HasMetallicRoughnessTexture",
                  HasTextureSlot(sourceData, "metallicRoughnessTexture") ? 1 : 0);
  instance.SetInt("u_HasNormalTexture", HasTextureSlot(sourceData, "normalTexture") ? 1 : 0);
  instance.SetInt("u_HasEmissiveTexture", HasTextureSlot(sourceData, "emissiveTexture") ? 1 : 0);
  instance.SetInt("u_HasOcclusionTexture", HasTextureSlot(sourceData, "occlusionTexture") ? 1 : 0);
}

void GLTFPBRMaterialTemplate::ApplyTextureSlots(MaterialInstance &instance,
                                                const MaterialSourceData &sourceData) const
{
  // 应用基础颜色纹理
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, "baseColorTexture")) {
    instance.SetTexture("u_BaseColorTexture", *slot);
  }

  // 应用金属粗糙度纹理（R:粗糙度, G:金属度）
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, "metallicRoughnessTexture")) {
    instance.SetTexture("u_MetallicRoughnessTexture", *slot);
  }

  // 应用法线纹理
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, "normalTexture")) {
    instance.SetTexture("u_NormalTexture", *slot);
  }

  // 应用自发光纹理
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, "emissiveTexture")) {
    instance.SetTexture("u_EmissiveTexture", *slot);
  }

  // 应用环境光遮蔽纹理
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, "occlusionTexture")) {
    instance.SetTexture("u_OcclusionTexture", *slot);
  }
}

void GLTFPBRMaterialTemplate::ApplyRenderProperties(MaterialInstance &instance,
                                                    const MaterialSourceData &sourceData) const
{
  // 获取渲染属性（使用源数据中的值，如果没有则使用默认值）
  AlphaMode alphaMode = sourceData.alphaMode;
  float alphaCutoff = sourceData.alphaCutoff;
  bool doubleSided = sourceData.doubleSided;

  // 设置透明度
  SetupAlphaBlending(instance, alphaMode, alphaCutoff);

  // 设置双面渲染
  instance.SetInt("u_DoubleSided", doubleSided ? 1 : 0);
}

void GLTFPBRMaterialTemplate::SetupAlphaBlending(MaterialInstance &instance,
                                                 AlphaMode alphaMode,
                                                 float alphaCutoff) const
{
  // 设置透明度模式
  instance.SetInt("u_AlphaMode", static_cast<int>(alphaMode));
  instance.SetFloat("u_AlphaCutoff", alphaCutoff);

  // 根据透明度模式设置混合参数
  switch (alphaMode) {
    case AlphaMode::OPAQUE:
      instance.SetInt("u_EnableAlphaTest", 0);
      instance.SetInt("u_EnableAlphaBlend", 0);
      break;
    case AlphaMode::MASK:
      instance.SetInt("u_EnableAlphaTest", 1);
      instance.SetInt("u_EnableAlphaBlend", 0);
      break;
    case AlphaMode::BLEND:
      instance.SetInt("u_EnableAlphaTest", 0);
      instance.SetInt("u_EnableAlphaBlend", 1);
      break;
  }
}
}  // namespace mite