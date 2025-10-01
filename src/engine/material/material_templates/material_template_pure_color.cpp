#include "material_template_pure_color.h"

namespace mite {

PureColorMaterialTemplate::PureColorMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                                     const glm::vec3 &color)
    : GBufferMaterialTemplate(std::move(shader)), m_Color(color)
{
  LOG_DEBUG(
      "PureColorMaterialTemplate created with color: ({}, {}, {})", color.r, color.g, color.b);
}

}  // namespace mite
