#include "ui_scenetree_panel.h"
#include "scene_core/scene_registry.h"

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

  // 订阅SceneNode选择事件
  m_EventSubscriptions.SubscribeImmediate<SceneNodeSelectedEvent>(
      BIND_DISPATCH_FN(OnSceneNodeSelected));
}

void SceneTreePanel::Update(float deltaTime) {}

void SceneTreePanel::Render()
{
  // 获取根节点并递归渲染
  auto rootNodes = m_SceneGraph.GetRootNodes();
  for (auto rootNode : rootNodes) {
    RenderNodeTreeRecursive(rootNode);
  }

  // 渲染空白区域用于接收拖拽行为
  auto dragDropTargetContent = [this](void *dropTargetData) {
    TreeNodeProps *props = static_cast<TreeNodeProps *>(dropTargetData);
    std::shared_ptr<SceneNode> dropNode = static_cast<std::shared_ptr<SceneNode> >(props->nodePtr);
    if (dropNode) {
      EventBus::Publish<SceneNodeParentChangeEvent>(SceneNodeParentChangeEvent(dropNode, nullptr));
    }
  };
  m_Renderer.RenderTreeVoid(dragDropTargetContent);

  // 如果没有根节点，显示提示信息
  if (rootNodes.empty()) {
    LabelProps labelProps;
    labelProps.translationKey = "scene is empty";
    m_Renderer.RenderLabel(labelProps);
  }
}

void SceneTreePanel::RenderNodeTreeRecursive(std::shared_ptr<SceneNode> node)
{
  if (!node)
    return;

  // 准备树节点属性
  TreeNodeProps treeNodeProps;
  treeNodeProps.nodePtr = node;
  treeNodeProps.translationKey = node->GetEntity().GetName();
  treeNodeProps.isSelect = (node == m_SelectedNode);
  treeNodeProps.isLeaf = node->IsLeaf();

  // 当作为拖拽的目标节点时，发布事件
  auto dragDropTargetContent = [this, node](void *dropTargetData) {
    TreeNodeProps *props = static_cast<TreeNodeProps *>(dropTargetData);
    std::shared_ptr<SceneNode> dropNode = props->nodePtr;
    if (dropNode) {
      EventBus::Publish<SceneNodeParentChangeEvent>(SceneNodeParentChangeEvent(dropNode, node));
    }
    else {
      EventBus::Publish<SceneNodeParentChangeEvent>(SceneNodeParentChangeEvent(dropNode, nullptr));
    }
  };

  // 当选择Item时发布事件
  auto itemSelectedContent = [this, node]() {
    EventBus::Publish<SceneNodeSelectedEvent>(SceneNodeSelectedEvent(node));
  };

  // 递归渲染子节点函数
  auto subitemRenderContent = [this, node]() {
    if (!node->IsLeaf()) {
      auto children = node->GetChildren();
      for (auto child : children) {
        RenderNodeTreeRecursive(child);
      }
    }
  };

  // 渲染此节点（内部更新treeNodeProps.isSelect）
  m_Renderer.RenderTreeNode(treeNodeProps,itemSelectedContent, dragDropTargetContent, subitemRenderContent);
}

void SceneTreePanel::RenameNode(std::shared_ptr<SceneNode> node, const std::string &name)
{
  if (!node)
    return;
  node->GetEntity().Rename(name);
}
void SceneTreePanel::OnSceneNodeSelected(SceneNodeSelectedEvent &event)
{
  if (event.GetSceneNode()) {
    // 节点可用，更新m_SelectedNode
    m_SelectedNode = event.GetSceneNode();

    // 不停止传播，SceneView需要同步更新PickedEntity
    event.SetResult(EventResult::Handled);
    return;
  }
  event.SetResult(EventResult::Failed);
  return;
}
}  // namespace mite