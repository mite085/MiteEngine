#include "material_template.h"

namespace mite {
// ---- PBRMaterialTemplate 实现 ----

PBRMaterialTemplate::PBRMaterialTemplate(std::shared_ptr<Shader> shader,
                                         const glm::vec3 &defaultAlbedo,
                                         float defaultRoughness,
                                         float defaultMetallic)
    : m_Shader(std::move(shader)),
      m_DefaultAlbedo(defaultAlbedo),
      m_DefaultRoughness(defaultRoughness),
      m_DefaultMetallic(defaultMetallic)
{
  // 参数合法性校验
  assert(m_Shader != nullptr && "PBRMaterialTemplate: Shader cannot be nullptr");
  assert((defaultRoughness >= 0.0f && defaultRoughness <= 1.0f) && "Roughness must be in range [0,1]");
  assert((defaultMetallic >= 0.0f && defaultMetallic <= 1.0f) && "Metallic must be in range [0,1]");
}

std::shared_ptr<MaterialInstance> PBRMaterialTemplate::CreateInstance() const 
{
  // 创建材质实例并应用默认参数
  auto instance = std::make_shared<MaterialInstance>(m_Shader);
  ApplyParameters(*instance);
  return instance;
}

void PBRMaterialTemplate::ApplyParameters(MaterialInstance &instance) const
{
  // 设置PBR基础参数
  instance.SetVector3("u_Albedo", m_DefaultAlbedo);
  instance.SetFloat("u_Roughness", m_DefaultRoughness);
  instance.SetFloat("u_Metallic", m_DefaultMetallic);

  // 绑定默认纹理（如果存在）
  for (const auto &[paramName, texture] : m_DefaultTextures) {
    instance.SetTexture(paramName, texture);
  }
}

void PBRMaterialTemplate::SetDefaultTexture(const std::string &paramName,
                                            std::shared_ptr<Texture> texture)
{
  assert(!paramName.empty() && "Texture name should not be empty");
  m_DefaultTextures[paramName] = std::move(texture);
}

// ---- TransparentMaterialTemplate 实现 ----

TransparentMaterialTemplate::TransparentMaterialTemplate(std::shared_ptr<Shader> shader,
                                                         float defaultAlpha)
    : PBRMaterialTemplate(std::move(shader)), m_DefaultAlpha(defaultAlpha)
{
  assert((defaultAlpha >= 0.0f && defaultAlpha <= 1.0f), "Alpha must be in range ");
}

std::shared_ptr<MaterialInstance> TransparentMaterialTemplate::CreateInstance() const
{
  auto instance = PBRMaterialTemplate::CreateInstance();
  ApplyParameters(*instance);  // 补充透明参数
  return instance;
}

void TransparentMaterialTemplate::ApplyParameters(MaterialInstance &instance) const
{
  // 先调用父类设置PBR参数
  PBRMaterialTemplate::ApplyParameters(instance);

  // 设置透明相关参数
  instance.SetFloat("u_Alpha", m_DefaultAlpha);
  instance.SetInt("u_EnableBlend", 1);  // 启用混合
}
};
