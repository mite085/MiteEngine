#include "ui_panel.h"

namespace mite {
UIPanel::UIPanel(const std::string &name) : m_Renderer(UIRender::Get())
{
  // 初始化PanelProps
  m_PanelProps.translationKey = name; // 设定name
  m_PanelProps.visible = true;        // 默认可见
  m_PanelProps.enabled = true;        // 默认开启
  m_PanelProps.resizable = true;      // 默认可调整大小
  m_PanelProps.scrollable = true;     // 默认可滚动
  m_PanelProps.collapsed = false;     // 默认不折叠标题
  m_PanelProps.bringToFront = false;  // 默认不置顶显示
  m_PanelProps.dockable = true;       // 默认支持停靠
  m_PanelProps.minSize = glm::vec2(0, 0);
  m_PanelProps.maxSize = glm::vec2(10000, 10000);
  LOG_DEBUG("Created UIPanel: {}", name);
}

void UIPanel::RenderPanel()
{
  // ImGui::Begin()
  if (m_Renderer.BeginPanel(m_PanelProps)) {
    // 执行具体渲染操作
    Render();

    // ImGui::End()
    m_Renderer.EndPanel();
  }
}
}  // namespace mite