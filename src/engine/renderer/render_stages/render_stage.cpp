#include "render_stage.h"

namespace mite {

RenderStage::RenderStage(const std::string &name)
    : m_Name(name)
{
  m_Logger = LoggerSystem::CreateModuleLogger("Mite RenderStage[" + name + "]");
  m_Logger->info("RenderStage '{}' created", name);
}

}  // namespace mite
