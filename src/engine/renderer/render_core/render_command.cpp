#include "render_core/render_command.h"
#include "render_opengl/opengl_command.h"

namespace mite {
RenderCommand &RenderCommand::Get()
{
  static OpenGLRenderCommand instance;
  return instance;
}

}  // namespace mite