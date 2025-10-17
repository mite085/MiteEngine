#ifndef MITE_VIEWPORT_PANEL_H
#define MITE_VIEWPORT_PANEL_H

#include "basic_event/render_event.h"
#include "ui_core/ui_render_props.h"
#include "ui_panel.h"
#include "ui_gizmo_overlay.h"

namespace mite {
/**
 * @brief 视口面板 - 仅负责显示和调整FrameBuffer尺寸
 *
 * 职责：
 * 1. 显示DisplayFrameBuffer的内容
 * 2. 根据面板尺寸调整MainFrameBuffer
 * 3. 同步相机宽高比
 * 4. 无事件处理，仅显示和Resize
 */
class ViewportPanel : public UIPanel {
 public:
  explicit ViewportPanel(const std::string &name);
  virtual ~ViewportPanel() = default;
  // UIPanel接口
  virtual void Update(float deltaTime) override;
  virtual void Render() override;

 private:
  // ==================== 私有方法 ====================
  void InitializePanelProps();
  void HandleSizeChange(const glm::vec2 &newSize);
  void OnRenderFinished(RuntimeTextureFinishedEvent &event);

  // ==================== Viewport输入上下文 ====================

  // ==================== OverLay显示 ====================
  std::unique_ptr<GizmoOverlay> m_GizmoOverlay;
  OverlayContext m_GizmoOverlayContext;

  // ==================== 纹理显示 ====================
  RuntimeTexturePtr m_DisplayTexture = nullptr;
  RuntimeTextureType m_DisplayTextureType = RuntimeTextureType::Lighting_Combined; 
  std::string m_DisplayTextureIdentify = "";

  // ==================== 状态管理 ====================
  ImageProps m_ImageProps;     // 图像渲染属性
  glm::vec2 m_CurrentSize;     // 当前面板尺寸
  glm::vec2 m_RequestedSize;   // 请求调整的尺寸
  SubscriptionGroup m_EventSubscriptions;
};
}  // namespace mite

#endif  // MITE_VIEWPORT_PANEL_H
