#ifndef MITE_UI_INSPECTOR_PANEL
#define MITE_UI_INSPECTOR_PANEL

#include "ui_core/ui_panel.h"
#include "scene_core/entity.h"
#include "scene_core/scene_registry.h"

namespace mite {
/**
 * @brief 实体属性检查与编辑面板
 * @note 动态显示当前选中实体的所有组件
 */
class InspectorPanel : public UIPanel {
 public:
  InspectorPanel(SceneRegistry & registry);

  void onRender() override;
  void OnEntitySelected(Entity entity);  // 事件回调

 private:
  // ---- 组件绘制工具函数 ----
  void DrawTransformComponent();
  void DrawMeshComponent();
  void DrawAddComponentMenu();

  // ---- SceneRegistry依赖注入 ----
  SceneRegistry &m_Registry;

  // ---- 当前选中实体 ----
  Entity m_CurrentEntity;
};
};

#endif
