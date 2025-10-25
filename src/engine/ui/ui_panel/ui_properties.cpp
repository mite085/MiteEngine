#include "ui_properties.h"

namespace mite {
// 格式：{枚举值, "显示名称"}
const EnumComboBoxList<Transform::EulerOrder, 6>
    PropertyTable<TransformComponent>::m_EulerOrderList = EnumComboBoxList(
        std::array{std::pair{Transform::EulerOrder::XYZ, "XYZ"},
                   std::pair{Transform::EulerOrder::XZY, "XZY"},
                   std::pair{Transform::EulerOrder::YXZ, "YXZ"},
                   std::pair{Transform::EulerOrder::YZX, "YZX"},
                   std::pair{Transform::EulerOrder::ZXY, "ZXY"},
                   std::pair{Transform::EulerOrder::ZYX, "ZYX"}});

void PropertyTable<TransformComponent>::Render(UIRender &render)
{
  // 绘制分隔符
  LabelProps spratorLabel;
  spratorLabel.translationKey = "editor.transform";
  render.RenderLabelSprator(spratorLabel);

  // 绘制表格
  TableProps tableProps;
  tableProps.columns = 2;          // 2列显示
  tableProps.showHeaders = false;  // 不显示表头
  tableProps.resizable = false;    // 不允许resize
  tableProps.borders = false;      // 不显示边框
  tableProps.rowBg = false;        // 无装饰
  tableProps.bordersInnerH = false;
  tableProps.bordersInnerV = false;
  tableProps.bordersOuterH = false;
  tableProps.bordersOuterV = false;

  render.RenderTable(tableProps, [&]() {
    // 新的一行绘制Location文本
    render.TableNextRow();
    LabelProps posLabel;
    posLabel.translationKey = "editor.location";
    render.RenderLabel(posLabel);
    // 同一行下一列绘制Location属性
    render.TableNextColume();
    DragFloat3Props posProps;
    posProps.value = m_Component.GetLocalTransform().GetPosition();
    if (render.RenderDragFloat3(posProps)) {
      m_Component.SetLocalTransform(
          [=](Transform &localtrans) { localtrans.SetPosition(posProps.value); });  // 修正本地变换
    }
    render.TableNextColume();

    // 新的一行绘制Rotation文本
    render.TableNextRow();
    LabelProps rotLabel;
    rotLabel.translationKey = "editor.rotation";
    render.RenderLabel(rotLabel);
    // 同一行下一列绘制Rotation属性
    render.TableNextColume();
    DragFloat3Props rotProps;
    rotProps.value = m_Component.GetLocalTransform().GetRotationEuler();
    if (render.RenderDragFloat3(rotProps)) {
      m_Component.SetLocalTransform([=](Transform &localtrans) {
        localtrans.SetRotationEuler(rotProps.value);
      });  // 修正本地变换
    }

    // 新的一行绘制RotationType文本
    render.TableNextRow();
    LabelProps rotTypeLabel;
    rotTypeLabel.translationKey = "editor.rotation_type";
    render.RenderLabel(rotTypeLabel);
    // 同一行下一列绘制RotationType选择
    render.TableNextColume();
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

    // 绘制Scale文本
    render.TableNextRow();
    LabelProps sclLabel;
    sclLabel.translationKey = "editor.scale";
    render.RenderLabel(sclLabel);
    // 同一行下一列绘制Rotation属性
    render.TableNextColume();
    DragFloat3Props sclProps;
    sclProps.value = m_Component.GetLocalTransform().GetScale();
    if (render.RenderDragFloat3(sclProps)) {
      m_Component.SetLocalTransform(
          [=](Transform &localtrans) { localtrans.SetScale(sclProps.value); });  // 修正本地变换
    }
  });
}

// 格式：{枚举值, "显示名称"}
const EnumComboBoxList<CameraProjectionType, 2> PropertyTable<CameraComponent>::m_CameraTypeList =
    EnumComboBoxList(
        std::array{std::pair{CameraProjectionType::PERSPECTIVE, "editor.camera_perspective"},
                   std::pair{CameraProjectionType::ORTHOGRAPHIC, "editor.camera_orthographic"}});

void PropertyTable<CameraComponent>::Render(UIRender &render)
{
  // 绘制分隔符
  LabelProps spratorLabel;
  spratorLabel.translationKey = "editor.camera";
  render.RenderLabelSprator(spratorLabel);

  // 绘制表格
  TableProps tableProps;
  tableProps.columns = 2;          // 2列显示
  tableProps.showHeaders = false;  // 不显示表头
  tableProps.resizable = false;    // 不允许resize
  tableProps.borders = false;      // 不显示边框
  tableProps.rowBg = false;        // 无装饰
  tableProps.bordersInnerH = false;
  tableProps.bordersInnerV = false;
  tableProps.bordersOuterH = false;
  tableProps.bordersOuterV = false;

  render.RenderTable(tableProps, [&]() {
    // 新的一行绘制RotationType文本
    render.TableNextRow();
    LabelProps cameraTypeLabel;
    cameraTypeLabel.translationKey = "editor.camera_type";
    render.RenderLabel(cameraTypeLabel);
    // 同一行下一列绘制RotationType选择
    render.TableNextColume();
    ComboboxProps cameraTypeProps;
    cameraTypeProps.itemTranslationKeys = m_CameraTypeList.GetTranslateKeys();  // 获取所有Keys
    cameraTypeProps.selectedIndex = m_CameraTypeList.GetIndex(
        m_Component.GetCamera()->GetProjectionType());  // 获取当前index
    if (render.RenderCombobox(cameraTypeProps)) {
      m_Component.SetProjectionType(m_CameraTypeList.GetEnumType(
          cameraTypeProps.selectedIndex));  // 使用选择后的index执行Set逻辑
    }
  });
}
}  // namespace mite