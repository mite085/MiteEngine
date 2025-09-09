#ifndef MITE_UI_VIEWPORT_PANEL
#define MITE_UI_VIEWPORT_PANEL

#include "basic_data/framebuffer.h"
#include "ui_widget/ui_panel.h"

namespace mite {
/**
 * @brief 3D视口面板，负责场景渲染和交互
 *
 * 核心功能：
 * 显示3D场景渲染结果（通过UIBackend后端绘制Framebuffer）
 */
class ViewportPanel : public UIPanel {
 public:

  explicit ViewportPanel(const std::string &title = "ViewPort");
  virtual ~ViewportPanel() override;

  /**
   * @brief 设置视口的帧缓冲对象
   * @param framebuffer 包含场景渲染结果的帧缓冲
   */
  void setFramebuffer(std::shared_ptr<FrameBuffer> framebuffer);

 private:
  // 渲染资源
  std::shared_ptr<FrameBuffer> m_Framebuffer = nullptr;  // 场景帧缓冲
};

}  // namespace mite

#endif
