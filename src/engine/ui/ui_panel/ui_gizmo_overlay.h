#ifndef MITE_GIZMO_OVERLAY_H
#define MITE_GIZMO_OVERLAY_H

#include "glm/glm.hpp"
#include "ui_overlay.h"

namespace mite {
/**
 * @brief Gizmo操作覆盖层 - 基于ImGuizmo的3D变换控件
 */
class GizmoOverlay : public UIOverlay {
 public:
  GizmoOverlay();
  virtual ~GizmoOverlay() = default;

  // UI Overlay接口
  void Update(float deltaTime) override;
  void Render(OverlayContext &context) override;

  // ==================== Gizmo状态控制 ====================
  void SetOperation(int operation);  // 设置操作类型：平移/旋转/缩放
  void SetMode(int mode);            // 设置模式：局部/世界
  void EnableSnap(bool snap) { m_UseSnap = snap; }      // 启用吸附
  void SetSnap(glm::vec3 snap) { m_SnapValue = snap; }  // 设定吸附值

  bool IsUsing() const { return m_IsUsing; }
  bool IsOver() const { return m_IsOver; }

 private:
  // ==================== Gizmo状态 ====================
  int m_CurrentOperation = 0;  // 操作类型
  int m_CurrentMode = 0;       // 坐标系模式

  // ==================== 交互状态 ====================
  bool m_IsUsing = false;  // 是否正在使用
  bool m_IsOver = false;   // 鼠标是否悬停在Gizmo上

  // ==================== 配置参数 ====================
  glm::vec2 m_ViewManipulateSize = {128, 128};  // Gizmo显示大小
  bool m_UseSnap = false;                       // 是否启用吸附
  glm::vec3 m_SnapValue =
      glm::vec3(1.0f);  // 吸附值(按照该值的倍数执行Translate/Rotate/Scale)
};
}  // namespace mite

#endif  // MITE_GIZMO_OVERLAY_H
