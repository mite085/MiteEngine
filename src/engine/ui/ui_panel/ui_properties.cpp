#include "ui_properties.h"

namespace mite {
void PropertyBase<TransformComponent>::Render(TransformComponent &component, UIRender &render)
{
  DragFloat3Props posProps;
  posProps.translationKey = "Position";
  posProps.value = component.GetLocalTransform().GetPosition();

  if (render.RenderDragFloat3(posProps)) {
    component.SetLocalTransform(
        [&](Transform localtrans) { localtrans.SetPosition(posProps.value); });
  }
}
}  // namespace mite