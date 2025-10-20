#include "scene_core/scene_registry.h"
#include "ui_scenetree_panel.h"

namespace mite {

SceneTreePanel::SceneTreePanel(SceneGraph &sceneGraph, const std::string &name)
    : UIPanel(name), m_SceneGraph(sceneGraph)
{
  // 初始化面板属性
  m_PanelProps.visible = true;
  m_PanelProps.enabled = true;
  m_PanelProps.movable = true;
  m_PanelProps.resizable = true;
  m_PanelProps.scrollable = true;
  m_PanelProps.minSize = glm::vec2(300, 400);
}

void SceneTreePanel::Update(float deltaTime)
{
  // 可以在这里处理动画或其他状态更新
  if (m_NeedsRefresh) {
    // 如果需要刷新，可以在下一帧渲染时处理
  }
}

void SceneTreePanel::Render()
{
  // 渲染节点操作工具栏
  RenderNodeToolbar();

  // 开始子窗口用于场景树内容
  ChildProps childProps;
  childProps.translationKey = "Scene Tree";
  childProps.size = glm::vec2(0, 0);  // 自动填充剩余空间
  childProps.border = true;
  childProps.scrollable = true;

  if (m_Renderer.BeginChild(childProps)) {
    // 获取根节点并递归渲染
    auto rootNodes = m_SceneGraph.GetRootNodes();
    for (auto *rootNode : rootNodes) {
      RenderNodeTreeRecursive(rootNode);
    }

    // 如果没有根节点，显示提示信息
    if (rootNodes.empty()) {
      LabelProps labelProps;
      labelProps.translationKey = "scene is empty";
      m_Renderer.RenderLabel(labelProps);
    }

    m_Renderer.EndChild();
  }
}

void SceneTreePanel::RenderNodeToolbar()
{
  // 创建新节点按钮
  ButtonProps createButtonProps;
  createButtonProps.translationKey = "create_node";
  createButtonProps.tooltip = "create new node";

  if (m_Renderer.RenderButton(createButtonProps)) {
    CreateNewNode(m_SelectedNode);
  }

  m_Renderer.SetSameLine();

  // 删除节点按钮（仅在选中节点时启用）
  m_Renderer.BeginDisabled(m_SelectedNode == nullptr);

  ButtonProps deleteButtonProps;
  deleteButtonProps.translationKey = "delete_node";
  deleteButtonProps.tooltip = "delete select node";

  if (m_Renderer.RenderButton(deleteButtonProps)) {
    DeleteNode(m_SelectedNode);
  }

  m_Renderer.EndDisabled();

  m_Renderer.SetSameLine();

  // 复制节点按钮（仅在选中节点时启用）
  m_Renderer.BeginDisabled(m_SelectedNode == nullptr);

  ButtonProps duplicateButtonProps;
  duplicateButtonProps.translationKey = "duplicate_node";
  duplicateButtonProps.tooltip = "copy select node";

  if (m_Renderer.RenderButton(duplicateButtonProps)) {
    DuplicateNode(m_SelectedNode);
  }

  m_Renderer.EndDisabled();

  m_Renderer.SetNewLine();
}

void SceneTreePanel::RenderNodeTreeRecursive(SceneNode *node)
{
  if (!node)
    return;

  // 应用过滤条件
  if (!m_SearchFilter.empty()) {
    // 这里需要获取节点名称，你可能需要在SceneNode中添加GetName()方法
    // std::string nodeName = node->GetName();
    // if (nodeName.find(m_SearchFilter) == std::string::npos) {
    //   return; // 不匹配过滤条件，跳过
    // }
  }

  if (m_ShowOnlyVisible && !node->IsWorldVisible()) {
    return;  // 过滤掉不可见节点
  }

  // 准备树节点属性
  TreeNodeProps treeNodeProps;
  treeNodeProps.translationKey = node->GetEntity().GetName();
  treeNodeProps.isSelect = (node == m_SelectedNode);
  treeNodeProps.isLeaf = node->IsLeaf();

  // 渲染树节点
  if (m_Renderer.RenderTreeNode(treeNodeProps, [this, node]() {
        // 树节点内容回调
        this->RenderSceneNode(node);
      }))
  {
    // 树节点展开/收起状态改变
  }

  // 处理节点选择
  if (treeNodeProps.isSelect && node != m_SelectedNode) {
    SetSelectedNode(node);
  }

  // 渲染子节点（如果展开）
  if (treeNodeProps.isExpand && !node->IsLeaf()) {
    auto children = node->GetChildren();
    for (auto *child : children) {
      RenderNodeTreeRecursive(child);
    }
  }
}

void SceneTreePanel::RenderSceneNode(SceneNode *node)
{
  if (!node)
    return;

  LabelProps labelProps;
  labelProps.translationKey = node->GetEntity().GetName();
  m_Renderer.RenderLabel(labelProps);

  // 右键菜单支持
  if (m_Renderer.IsPanelHovered() && m_Renderer.IsPanelFocused()) {
    // 检测右键点击，打开上下文菜单
    // TODO: 在UIRender中添加右键点击检测功能
  }
}

void SceneTreePanel::SetSelectedNode(SceneNode *node)
{
  if (m_SelectedNode != node) {
    m_SelectedNode = node;
    // 可以在这里触发选中事件
  }
}

SceneNode *SceneTreePanel::GetSelectedNode() const
{
  return m_SelectedNode;
}

void SceneTreePanel::RefreshTree()
{
  m_NeedsRefresh = true;
}

void SceneTreePanel::CreateNewNode(SceneNode *parent)
{
  // TODO: 创建新的ECS实体和场景节点
  //m_SceneGraph.CreateNode(parent);
  m_NeedsRefresh = true;
}

void SceneTreePanel::DeleteNode(SceneNode *node)
{
  if (!node)
    return;

  // 如果删除的是选中的节点，清空选中状态
  if (node == m_SelectedNode) {
    m_SelectedNode = nullptr;
  }

  // TODO: 删除节点
  //m_SceneGraph.DestroyNode(node);
  m_NeedsRefresh = true;
}

void SceneTreePanel::DuplicateNode(SceneNode *node)
{
  if (!node)
    return;

  // TODO: 复制节点逻辑
  m_NeedsRefresh = true;
}

void SceneTreePanel::RenameNode(SceneNode *node)
{
  if (!node)
    return;

  m_RenameTarget = node;
  // m_RenameBuffer = node->GetName(); // 需要SceneNode支持获取名称
}

}  // namespace mite
