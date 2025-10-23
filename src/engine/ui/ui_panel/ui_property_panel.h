#ifndef MITE_PROPERTY_PANEL_H
#define MITE_PROPERTY_PANEL_H

#include "basic_event/render_event.h"
#include "ui_properties.h"
#include "ui_panel.h"

namespace mite {
/**
 * @brief 属性页面板 - 负责组件数据的编辑与修改
 */
class PropertyPanel : public UIPanel {
 public:
  /**
   * @brief 构造函数
   * @param name 面板名称
   * @param sceneGraph 场景图引用（依赖注入）
   * @param sceneRegistry 场景注册表引用（依赖注入）
   */
  PropertyPanel(SceneGraph &sceneGraph, SceneRegistry &sceneRegistry, const std::string &name);
  ~PropertyPanel() override = default;

  // ==================== 核心接口实现 ====================
  void Update(float deltaTime) override {}
  void Render() override;

 private:
  /**
   * @brief 订阅节点选择事件，基于选择的值更新m_SelectedNode
   */
  void OnSceneNodeSelected(SceneNodeSelectedEvent &event);


  template<typename T> void RenderProperty() {
    static_assert(std::is_base_of<Component T>::value, "T Must be derived from component");
    if (!m_SelectedNode)
      return;

    if (m_SceneRegistry.HasComponent<T>(m_SelectedNode->GetEntity())) {
      auto component = m_SceneRegistry.GetComponent<T>(m_SelectedNode->GetEntity());
      m_
    }
	  
  }

  // ==================== 成员变量 ====================
  SceneGraph &m_SceneGraph;
  SceneRegistry &m_SceneRegistry;
  SceneNode *m_SelectedNode = nullptr;  // 当前选中的节点

  SubscriptionGroup m_EventSubscriptions;  // 事件订阅
};
}  // namespace mite

#endif  // MITE_PROPERTY_PANEL_H