#ifndef MITE_UI_VIEWPORT_PANEL
#define MITE_UI_VIEWPORT_PANEL

#include "ui_core/ui_panel.h"
#include "scene_core/entity.h"

namespace mite {
/**
 * @brief 3D场景视口渲染与交互面板
 * @note 负责处理相机控制、Gizmo操作等核心交互逻辑
 */
class ViewportPanel : public UIPanel<ViewportPanel> {
 public:
  ViewportPanel();

 protected:
  void DrawContent() override;
  void OnAttach() override;
  void OnUpdate(float dt) override;

 private:
  //=== 视口状态 ===//
  void HandleCameraControl();             // 鼠标相机控制
  void DrawGizmo(Entity selected);  // 绘制变换Gizmo
  void ProcessViewportResize();           // 处理视口尺寸变化

  //=== 渲染资源 ===//
  struct {
    uint32_t width = 0;
    uint32_t height = 0;
    Renderer::Framebuffer fbo;  // 关联Renderer模块的FBO
  } m_viewport;

  //=== 交互状态 ===//
  float m_cameraYaw = -90.0f;  // 相机欧拉角
  float m_cameraPitch = 0.0f;
  bool m_isViewportHovered = false;
};
};

#endif
