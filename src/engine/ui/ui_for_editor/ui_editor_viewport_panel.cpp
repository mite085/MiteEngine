#include "ui_editor_viewport_panel.h"
#include "ui_imgui_backend/ui_imgui_control_renderer.h"
#include "ui_core/ui_localization.h"

namespace mite {
ViewportPanel::ViewportPanel(const std::string &title) : UIPanel(title) {}

ViewportPanel::~ViewportPanel() {}

void ViewportPanel::Render()
{
  // 设置视口窗口样式(无内边距)
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin(UILocalization::Get().Translate(m_Title).c_str(), &m_Visible);

  // 获取视口可用区域大小
  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  m_ViewportSize = {contentSize.x, contentSize.y};
   
  // 将FrameBuffer填满ViewPort
  IMGUI_IMAGE(m_Framebuffer->GetColorAttachmentID(), m_ViewportSize);

  ImGui::End();
  ImGui::PopStyleVar();
}

void ViewportPanel::setFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
  m_Framebuffer = framebuffer;
}


}  // namespace mite