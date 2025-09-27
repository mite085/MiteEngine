#include "material_template_pure_color.h"

namespace mite {
// ---- BasicMaterialTemplate 实现 ----
PureColorMaterialTemplate::PureColorMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                             const glm::vec3 &color)
    : MaterialTemplate(shader), m_Color(color)
{
}

std::shared_ptr<MaterialInstance> PureColorMaterialTemplate::CreateInstance(
    const MaterialSourceData &sourceData) const
{
  auto instance = std::make_unique<MaterialInstance>(m_Shader);

  ApplyParameters(*instance, sourceData);
  return std::move(instance);
}

void PureColorMaterialTemplate::ApplyDefaultParams(MaterialInstance &instance) const
{
  instance.SetVector3("u_Color", m_Color);
}

void PureColorMaterialTemplate::ApplyParameters(MaterialInstance &instance,
                                                const MaterialSourceData &sourceData) const
{
  glm::vec3 baseColor = GetParameter(sourceData, "baseColorFactor", m_Color);
}



};  // namespace mite