#ifndef MITE_UI_INSPECTOR_PANEL
#define MITE_UI_INSPECTOR_PANEL

#include "ui_core/ui_panel.h"
#include "scene_core/entity.h"

namespace mite {
/**
 * @brief 实体属性检查与编辑面板
 * @note 动态显示当前选中实体的所有组件
 */
class InspectorPanel : public UIPanel<InspectorPanel> {
 public:
  InspectorPanel();

 protected:
  void DrawContent() override;
  void OnEntitySelected(entt::entity entity);  // 事件回调

 private:
  // 组件绘制工具函数
  void DrawTransformComponent(entt::registry &registry, entt::entity entity);
  void DrawMeshComponent(entt::registry &registry, entt::entity entity);
  void DrawAddComponentMenu();

  entt::entity m_currentEntity = entt::null;  // 当前选中实体
};
};

#endif
