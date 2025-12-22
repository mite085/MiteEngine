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
    : IPropertyTable("editor.transform"), m_Component(component) {}

void PropertyTable<TransformComponent>::Render(UIRender &render) {
  // 局部空间位置
  RenderLabelItemRow(render, "editor.transform_location", [&]() {
    m_PosProps.translationKey = "math.meter";
    m_PosProps.value = m_Component.GetLocalTransform().GetPosition();
    if (render.RenderDragFloat3(m_PosProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetPosition(m_PosProps.value);
      });  // 修正本地变换
    }
  });

  // 局部空间旋转
  RenderLabelItemRow(render, "editor.transform_rotation", [&]() {
    m_RotProps.translationKey = "math.degree";
    m_RotProps.value = m_Component.GetLocalTransform().GetRotationEuler();
    if (render.RenderDragFloat3(m_RotProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetRotationEuler(m_RotProps.value);
      });  // 修正本地变换
    }
  });

  // 局部空间旋转类型（欧拉）
  RenderLabelItemRow(render, "editor.transform_rotation_type", [&]() {
    m_RotTypeProps.itemTranslationKeys =
        m_EulerOrderList.GetTranslateKeyList();  // 获取所有Keys
    m_RotTypeProps.selectedIndex = m_EulerOrderList.GetIndex(
        m_Component.GetLocalTransform().GetRotationOrder());  // 获取当前index
    if (render.RenderCombobox(m_RotTypeProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetRotationOrder(m_EulerOrderList.GetEnumType(
            m_RotTypeProps.selectedIndex));  // 使用选择后的index执行Set逻辑
      });
    }
  });

  // 局部空间缩放
  RenderLabelItemRow(render, "editor.transform_scale", [&]() {
    m_SclProps.value = m_Component.GetLocalTransform().GetScale();
    m_SclProps.dragSpeed = 0.01f;

    // 注意：
    // Transform类同时存储了TRS +
    // Matrix，但Gizmo使用了Imguizmo的，仅管理Matrix。
    // Gizmo操作矩阵之后使用glm::decompose分解到TRS，单个scale分量为负会导致分解
    // 后的三个scale分量均为负；目前没有好的解决方式。仅限制Scale在正数区间
    //
    // 此处使用和显示格式"%.3f"匹配的最小正数，确保不会存在负的scale。
    m_SclProps.minValue = 0.001f;
    if (render.RenderDragFloat3(m_SclProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetScale(m_SclProps.value);
      });  // 修正本地变换
    }
  });
}
}  // namespace mite