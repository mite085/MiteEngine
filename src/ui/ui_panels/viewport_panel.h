#ifndef MITE_UI_VIEWPORT_PANEL
#define MITE_UI_VIEWPORT_PANEL

#include "ui_core/ui_panel.h"
#include "scene_core/entity.h"
#include "ImGuizmo.h"
#include "renderer.h"

namespace mite {
/**
 * @brief 3D视口面板，支持场景导航和Gizmo操作
 * @note 依赖ImGuizmo库实现变换工具
 */
class ViewportPanel : public UIPanel<ViewportPanel> {
 public:
  ViewportPanel(Renderer &renderer);

  // 设置当前选中实体（供InspectorPanel调用）
  void SetSelectedEntity(Entity entity)
  {
    m_selectedEntity = entity;
  }

  void DrawContent() override;
  void OnAttach() override;
  void OnUpdate(float dt) override;

 private:
  // ---- Renderer依赖注入 ----
  Renderer &m_renderer;

  // ---- Gizmo操作 ----
  void DrawGizmoToolbar();  // 绘制Gizmo模式选择工具栏
  void HandleGizmo();       // 处理Gizmo变换操作

  // ---- 视口控制 ----
  void UpdateCamera(float dt);  // 相机控制逻辑
  void CalculateViewMatrix();   // 计算视图矩阵

  // ---- 状态 ----
  Entity m_selectedEntity;           // 当前选中的ECS实体
  ImGuizmo::OPERATION m_gizmoOp{ImGuizmo::TRANSLATE};  // 当前Gizmo操作模式
  ImGuizmo::MODE m_gizmoMode{ImGuizmo::LOCAL};         // 坐标系模式

  // 相机参数
  glm::vec3 m_cameraPos{0, 0, 5};
  glm::vec3 m_cameraFront{0, 0, -1};
  float m_cameraSpeed{2.5f};

  // 矩阵缓存
  glm::mat4 m_viewMatrix = glm::mat4();
  glm::mat4 m_projMatrix = glm::mat4();
};
};

#endif
