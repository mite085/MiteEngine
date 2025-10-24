#include "ui_properties.h"

namespace mite {
void PropertyBase<TransformComponent>::Render(UIRender &render)
{
  // 绘制Position文本
  LabelProps posLabel;
  posLabel.translationKey = "Position";
  render.RenderLabel(posLabel);

  // 同一行绘制Position属性
  render.SetSameLine();
  DragFloat3Props posProps;
  posProps.value = m_Component.GetLocalTransform().GetPosition();
  if (render.RenderDragFloat3(posProps)) {
    m_Component.SetLocalTransform(
        [=](Transform &localtrans) { localtrans.SetPosition(posProps.value); });
  }

  // 绘制Rotation文本
  LabelProps rotLabel;
  rotLabel.translationKey = "Rotation";
  render.RenderLabel(rotLabel);

  // 同一行绘制Rotation属性
  render.SetSameLine();
  DragFloat3Props rotProps;
  rotProps.value = m_Component.GetLocalTransform().GetRotationEuler();
  if (render.RenderDragFloat3(rotProps)) {
    m_Component.SetLocalTransform(
        [=](Transform &localtrans) { localtrans.SetRotationEuler(rotProps.value); });
  }

}
}  // namespace mite