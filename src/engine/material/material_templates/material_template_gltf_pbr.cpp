#include "material_template_gltf_pbr.h"

namespace mite {

GLTFPBRMaterialTemplate::GLTFPBRMaterialTemplate(std::shared_ptr<OpenGLShader> shader)
    : MaterialTemplate(std::move(shader))
{
  LOG_DEBUG("GLTFPBRMaterialTemplate created");
}


}  // namespace mite
