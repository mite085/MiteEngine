#include "ui_panel.h"
#include <imgui.h>
#include "ui_panels/inspector_panel.h"
#include "ui_panels/viewport_panel.h"

namespace mite {
template<typename DerivedPanel>
UIPanel<DerivedPanel>::UIPanel(const std::string &name) : m_name(name)
{
  // 初始化默认样式
  m_windowFlags |= ImGuiWindowFlags_NoCollapse;
}

template<typename DerivedPanel> void UIPanel<DerivedPanel>::Draw()
{
  if (!m_visible)
    return;

  // 首次绘制时调用OnAttach
  if (m_firstDraw) {
    static_cast<DerivedPanel *>(this)->OnAttach();
    m_firstDraw = false;
  }

  // 应用自定义窗口样式
  BeginWindowStyle();

  // 开始绘制面板
  if (ImGui::Begin(m_name.c_str(), &m_visible, m_windowFlags)) {
    // 调用子类具体内容绘制
    static_cast<DerivedPanel *>(this)->DrawContent();
  }
  ImGui::End();

  // 恢复样式
  EndWindowStyle();
}

template<typename DerivedPanel> void UIPanel<DerivedPanel>::BeginWindowStyle()
{
  // 保存当前样式上下文
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

  // 设置窗口默认大小（首次生效）
  if (m_firstDraw) {
    ImGui::SetNextWindowSize(m_defaultSize, ImGuiCond_FirstUseEver);
  }
}

template<typename DerivedPanel> void UIPanel<DerivedPanel>::EndWindowStyle()
{
  // 恢复样式
  ImGui::PopStyleVar(2);
}

//=== 显式实例化常用面板类型 ===//
template class UIPanel<ViewportPanel>;   // 视口面板
template class UIPanel<InspectorPanel>;  // 属性检查器
};
