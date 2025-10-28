#include "ui_property_panel.h"
#include "ui_property/ui_property_camera.h"
#include "ui_property/ui_property_material.h"
#include "ui_property/ui_property_transform.h"
#include "ui_property/ui_property_light.h"
#include "ui_property/ui_property_bounding_volume.h"
#include "ui_property/ui_property_mesh.h"
#include "ui_property/ui_property_visibility.h"

namespace mite {
PropertyPanel::PropertyPanel(SceneRegistry &sceneRegistry, const std::string &name)
    : m_SceneRegistry(sceneRegistry), UIPanel(name)
{
  m_EventSubscriptions.SubscribeImmediate<SceneNodeSelectedEvent>(
      BIND_DISPATCH_FN(OnSceneNodeSelected));
}

void PropertyPanel::Render()
{
  if (m_SelectedNode) {
    RenderProperty<TransformComponent>();
    RenderProperty<CameraComponent>();
    RenderProperty<MaterialComponent>();
    RenderProperty<LightComponent>();
    RenderProperty<BoundingVolumeComponent>();
    RenderProperty<VisibilityComponent>();

    // 网格体显示在最下面
    RenderProperty<MeshComponent>();
  }
  else {
    // 不存在选中节点的占位显示
    LabelProps placeholderProps;
    placeholderProps.visible = true;
    placeholderProps.translationKey = "Invalid Selected Item";
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