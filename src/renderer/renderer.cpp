#include "renderer.h"

namespace mite {
Renderer::Renderer(std::shared_ptr<AssetManager> assetManager):assetManager_(assetManager) {}
void Renderer::SetClearColor(const glm::vec4 &color) {}
void Renderer::SetViewport(uint32_t width, uint32_t height) {}
}  // namespace mite