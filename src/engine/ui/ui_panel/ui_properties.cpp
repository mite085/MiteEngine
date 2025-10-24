#include "ui_properties.h"

namespace mite {
void PropertyBase<TransformComponent>::Render(UIRender &render)
{
  DragFloat3Props posProps;
  posProps.translationKey = "Position";
  posProps.value = m_Component.GetLocalTransform().GetPosition();

  if (render.RenderDragFloat3(posProps)) {
    m_Component.SetLocalTransform(
        [&](Transform localtrans) { localtrans.SetPosition(posProps.value); });
  }
}

}  // namespace mite