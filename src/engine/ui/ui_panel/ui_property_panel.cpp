#include "ui_property_panel.h"

namespace mite {
PropertyPanel::PropertyPanel(SceneGraph &sceneGraph,
                             SceneRegistry &sceneRegistry,
                             const std::string &name)
    : m_SceneGraph(sceneGraph), m_SceneRegistry(sceneRegistry), UIPanel(name)
{
  m_EventSubscriptions.SubscribeImmediate<SceneNodeSelectedEvent>(
      BIND_DISPATCH_FN(OnSceneNodeSelected));
}

void PropertyPanel::Render()
{
  if (m_SelectedNode) {
    RenderProperty<TransformComponent>();

  }
  else {
    // 不存在选中节点的占位显示
    LabelProps placeholderProps;
    placeholderProps.visible = true;
    placeholderProps.fallbackText = "Invalid Selected Item";
    m_Renderer.RenderLabel(placeholderProps);
  }
}

void PropertyPanel::OnSceneNodeSelected(SceneNodeSelectedEvent &event)
{
  if (event.GetSceneNode())
    m_SelectedNode = event.GetSceneNode();

  event.SetResult(EventResult::Handled);
  return;
}
}  // namespace mite