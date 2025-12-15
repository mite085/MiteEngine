#ifndef MITE_SCENE_TREE_PANEL_H
#define MITE_SCENE_TREE_PANEL_H

#include "scene_graph.h"
#include "ui_panel.h"

namespace mite {

/**
 * @brief 场景树面板 - 显示和管理场景节点层级结构
 */
class SceneTreePanel : public UIPanel {
 public:
  /**
   * @brief 构造函数
   * @param name 面板名称
   * @param sceneGraph 场景图引用（依赖注入）
   */
  SceneTreePanel(SceneGraph &sceneGraph, const std::string &name);
  ~SceneTreePanel() override = default;

  // ==================== 核心接口实现 ====================
  void Update(float deltaTime) override;
  void Render() override;

 private:
  // ==================== 私有方法 ====================
  /**
   * @brief 递归渲染节点树
   * @param node 当前节点
   */
  void RenderNodeTreeRecursive(std::shared_ptr<SceneNode> node);
  /**
   * @brief 重命名节点(由属性页执行该功能，SceneTree不承担)
   * @param node 要重命名的节点
   */
  void RenameNode(std::shared_ptr<SceneNode> node, const std::string &name);
  /**
   * @brief 消费节点选中事件，更新当前选中的节点
   */
  void OnSceneNodeSelected(SceneNodeSelectedEvent &event);

 private:
  SceneGraph &m_SceneGraph;             // 场景图引用
  std::shared_ptr<SceneNode> m_SelectedNode = nullptr;  // 当前选中的节点
  SubscriptionGroup m_EventSubscriptions; // 事件订阅
};

}  // namespace mite

#endif  // MITE_SCENE_TREE_PANEL_H
