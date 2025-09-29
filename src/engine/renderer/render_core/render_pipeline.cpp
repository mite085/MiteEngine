#include "render_pipeline.h"

namespace mite {

RenderPipeline::RenderPipeline()
    : m_Logger(LoggerSystem::CreateModuleLogger("Mite Render Pipeline"))
{
  m_Logger->info("Render Pipeline created");
}

void RenderPipeline::AddStage(std::unique_ptr<RenderStage> stage)
{
  if (!stage) {
    m_Logger->warn("Attempted to add null stage to pipeline");
    return;
  }

  m_Stages.push_back(std::move(stage));
  m_Logger->debug("Added stage: {}", m_Stages.back()->GetName());
}

void RenderPipeline::SetStageEnabled(const std::string &stageName, bool enabled)
{
  for (auto &stage : m_Stages) {
    if (stage->GetName() == stageName) {
      stage->SetEnabled(enabled);
      m_Logger->debug("Set stage {} enabled: {}", stageName, enabled);
      return;
    }
  }
  m_Logger->warn("Stage not found: {}", stageName);
}

RenderStage *RenderPipeline::GetStage(const std::string &stageName) const
{
  for (auto &stage : m_Stages) {
    if (stage->GetName() == stageName) {
      return stage.get();
    }
  }
  return nullptr;
}

}  // namespace mite
