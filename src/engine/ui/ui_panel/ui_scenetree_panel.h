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

  // ==================== 场景树操作 ====================
  /**
   * @brief 设置选中的场景节点
   * @param node 要选中的节点
   */
  void SetSelectedNode(SceneNode *node);
  /**
   * @brief 获取当前选中的场景节点
   * @return 选中的节点指针（可能为nullptr）
   */
  SceneNode *GetSelectedNode() const;
  /**
   * @brief 刷新场景树（当场景结构发生变化时调用）
   */
  void RefreshTree();

 private:
  // ==================== 渲染辅助方法 ====================
  /**
   * @brief 渲染单个场景节点及其子节点
   * @param node 要渲染的节点
   */
  void RenderSceneNode(SceneNode *node);
  /**
   * @brief 渲染节点操作工具栏
   */
  void RenderNodeToolbar();

  /**
   * @brief 递归渲染节点树
   * @param node 当前节点
   */
  void RenderNodeTreeRecursive(SceneNode *node);

  // ==================== 节点操作 ====================
  /**
   * @brief 创建新节点
   * @param parent 父节点（nullptr表示根节点）
   */
  void CreateNewNode(SceneNode *parent = nullptr);
  /**
   * @brief 删除节点
   * @param node 要删除的节点
   */
  void DeleteNode(SceneNode *node);
  /**
   * @brief 复制节点
   * @param node 要复制的节点
   */
  void DuplicateNode(SceneNode *node);
  /**
   * @brief 重命名节点
   * @param node 要重命名的节点
   */
  void RenameNode(SceneNode *node);

 private:
  SceneGraph &m_SceneGraph;             // 场景图引用
  SceneNode *m_SelectedNode = nullptr;  // 当前选中的节点

  // 状态管理
  bool m_NeedsRefresh = true;           // 是否需要刷新树结构
  std::string m_RenameBuffer;           // 重命名缓冲区
  SceneNode *m_RenameTarget = nullptr;  // 正在重命名的节点

  // 过滤和搜索
  std::string m_SearchFilter;      // 搜索过滤器
  bool m_ShowOnlyVisible = false;  // 是否只显示可见节点
};

}  // namespace mite

#endif  // MITE_SCENE_TREE_PANEL_H
