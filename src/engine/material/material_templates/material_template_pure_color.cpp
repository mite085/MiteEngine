#include "material_template_pure_color.h"

namespace mite {
EmissionMaterialTemplate::EmissionMaterialTemplate(
    const glm::vec3 &emissionColor)
    : MaterialTemplate(), m_EmissionColor(emissionColor) {
  LOG_DEBUG("EmissionMaterialTemplate created with color: ({}, {}, {})",
            emissionColor.r, emissionColor.g, emissionColor.b);
}
}  // namespace mite