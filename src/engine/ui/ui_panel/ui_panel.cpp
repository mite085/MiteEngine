#include "ui_panel.h"

namespace mite {

UIPanel::UIPanel(const std::string &name) : m_Renderer(UIRender::Get())
{
  // 初始化PanelProps
  m_PanelProps.elementId = UUIDGenerator::Generate();
  m_PanelProps.visible = true;
  m_PanelProps.enabled = true;
  m_PanelProps.fallbackText = name;
  m_PanelProps.movable = true;
  m_PanelProps.resizable = true;
  m_PanelProps.scrollable = true;
  m_PanelProps.collapsed = false;
  m_PanelProps.bringToFront = false;
  m_PanelProps.minSize = glm::vec2(0, 0);
  m_PanelProps.maxSize = glm::vec2(10000, 10000);
  LOG_DEBUG("Created UIPanel: {}", name);
}

void UIPanel::RenderPanel() {
  if (m_PanelProps.visible) {
    // ImGui::Begin()
    m_Renderer.BeginPanel(m_PanelProps);

    // 执行具体渲染操作
    Render();

    // ImGui::End()
    m_Renderer.EndPanel();
  }
}

}  // namespace mite
