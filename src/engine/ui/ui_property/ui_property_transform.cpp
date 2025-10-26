#include "ui_property_transform.h"

namespace mite {
const EnumComboBoxList<Transform::EulerOrder, 6>
    PropertyTable<TransformComponent>::m_EulerOrderList = EnumComboBoxList(
        std::array{std::pair{Transform::EulerOrder::XYZ, "XYZ"},
                   std::pair{Transform::EulerOrder::XZY, "XZY"},
                   std::pair{Transform::EulerOrder::YXZ, "YXZ"},
                   std::pair{Transform::EulerOrder::YZX, "YZX"},
                   std::pair{Transform::EulerOrder::ZXY, "ZXY"},
                   std::pair{Transform::EulerOrder::ZYX, "ZYX"}});

PropertyTable<TransformComponent>::PropertyTable(TransformComponent &component)
    : IPropertyTable("editor.transform"), m_Component(component)
{
}

void PropertyTable<TransformComponent>::Render(UIRender &render)
{
  // 局部空间位置
  RenderLabelItemRow(render ,"editor.transform_location", [&]() {
    DragFloat3Props posProps;
    posProps.translationKey = "math.meter";
    posProps.value = m_Component.GetLocalTransform().GetPosition();
    if (render.RenderDragFloat3(posProps)) {
      m_Component.SetLocalTransform(
          [=](Transform &localtrans) { localtrans.SetPosition(posProps.value); });  // 修正本地变换
    }
  });

  // 局部空间旋转
  RenderLabelItemRow(render ,"editor.transform_rotation", [&]() {
    DragFloat3Props rotProps;
    rotProps.translationKey = "math.degree";
    rotProps.value = m_Component.GetLocalTransform().GetRotationEuler();
    if (render.RenderDragFloat3(rotProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetRotationEuler(rotProps.value);
      });  // 修正本地变换
    }
  });

  // 局部空间旋转类型（欧拉）
  RenderLabelItemRow(render ,"editor.transform_rotation_type", [&]() {
    ComboboxProps rotTypeProps;
    rotTypeProps.itemTranslationKeys = m_EulerOrderList.GetTranslateKeys();  // 获取所有Keys
    rotTypeProps.selectedIndex = m_EulerOrderList.GetIndex(
        m_Component.GetLocalTransform().GetRotationOrder());  // 获取当前index
    if (render.RenderCombobox(rotTypeProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetRotationOrder(m_EulerOrderList.GetEnumType(
            rotTypeProps.selectedIndex));  // 使用选择后的index执行Set逻辑
      });
    }
  });

  // 局部空间缩放
  RenderLabelItemRow(render ,"editor.transform_scale", [&]() {
    DragFloat3Props sclProps;
    sclProps.value = m_Component.GetLocalTransform().GetScale();
    sclProps.speed = 0.01f;

    // 注意：
    // Transform类同时存储了TRS + Matrix，但Gizmo使用了Imguizmo的，仅管理Matrix。
    // Gizmo操作矩阵之后使用glm::decompose分解到TRS，单个scale分量为负会导致分解
    // 后的三个scale分量均为负；目前没有好的解决方式。仅限制Scale在正数区间
    //
    // 此处使用和显示格式"%.3f"匹配的最小正数，确保不会存在负的scale。
    sclProps.minValue = 0.001f;
    if (render.RenderDragFloat3(sclProps)) {
      m_Component.SetLocalTransform(
          [=](Transform &localtrans) { localtrans.SetScale(sclProps.value); });  // 修正本地变换
    }
  });  
}
}  // namespace mite