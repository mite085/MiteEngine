#include "ui_property_camera.h"

namespace mite {
const EnumComboBoxList<CameraProjectionType, 2> PropertyTable<CameraComponent>::m_CameraTypeList =
    EnumComboBoxList(
        std::array{std::pair{CameraProjectionType::PERSPECTIVE, "editor.camera_perspective"},
                   std::pair{CameraProjectionType::ORTHOGRAPHIC, "editor.camera_orthographic"}});

PropertyTable<CameraComponent>::PropertyTable(CameraComponent &component)
    : IPropertyTable("editor.camera"), m_Component(component)
{

}

void PropertyTable<CameraComponent>::Render(UIRender &render)
{
  std::shared_ptr<Camera> camera = m_Component.GetCamera();
  if (!camera)
    return;

  RenderLabelItemRow(render, "editor.camera_type", [&]() {
    m_CameraTypeProps.itemTranslationKeys = m_CameraTypeList.GetTranslateKeyList();  // 获取所有Keys
    m_CameraTypeProps.selectedIndex = m_CameraTypeList.GetIndex(
        camera->GetProjectionType());  // 获取当前index
    if (render.RenderCombobox(m_CameraTypeProps)) {
      camera->SetProjectionType(m_CameraTypeList.GetEnumType(
          m_CameraTypeProps.selectedIndex));  // 使用选择后的index执行Set逻辑
    }
  });


  if (camera->GetProjectionType() == CameraProjectionType::PERSPECTIVE) {
    RenderLabelItemRow(render, "editor.camera_fov", [&]() {
      m_FovProps.translationKey = "math.degree";
      m_FovProps.value = camera->GetFOV();
      m_FovProps.minValue = Camera::FovMin();
      m_FovProps.maxValue = Camera::FovMax();
      m_FovProps.dragSpeed = 0.01f;
      if (render.RenderDragFloat(m_FovProps)) {
        camera->SetFov(m_FovProps.value);
      }
    });
  }
  else {
    RenderLabelItemRow(render, "editor.camera_ortho_size", [&]() {
      m_FovProps.translationKey = "math.meter";
      m_FovProps.value = camera->GetOrthoSize();
      m_FovProps.minValue = Camera::OrthoSizeMin();
      m_FovProps.maxValue = Camera::OrthoSizeMax();
      m_FovProps.dragSpeed = 0.1f;
      if (render.RenderDragFloat(m_FovProps)) {
        camera->SetOrthoSize(m_FovProps.value);
      }
    });
  }
}
}  // namespace mite