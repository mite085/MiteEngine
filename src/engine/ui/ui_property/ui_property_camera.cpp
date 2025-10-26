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
    ComboboxProps cameraTypeProps;
    cameraTypeProps.itemTranslationKeys = m_CameraTypeList.GetTranslateKeys();  // 获取所有Keys
    cameraTypeProps.selectedIndex = m_CameraTypeList.GetIndex(
        camera->GetProjectionType());  // 获取当前index
    if (render.RenderCombobox(cameraTypeProps)) {
      camera->SetProjectionType(m_CameraTypeList.GetEnumType(
          cameraTypeProps.selectedIndex));  // 使用选择后的index执行Set逻辑
    }
  });


  if (camera->GetProjectionType() == CameraProjectionType::PERSPECTIVE) {
    RenderLabelItemRow(render, "editor.camera_fov", [&]() {
      DragFloatProps fovProps;
      fovProps.translationKey = "math.degree";
      fovProps.value = camera->GetFOV();
      fovProps.minValue = Camera::FovMin();
      fovProps.maxValue = Camera::FovMax();
      fovProps.speed = 0.01f;
      if (render.RenderDragFloat(fovProps)) {
        camera->SetFov(fovProps.value);
      }
    });
  }
  else {
    RenderLabelItemRow(render, "editor.camera_ortho_size", [&]() {
      DragFloatProps orthoSizeProps;
      orthoSizeProps.translationKey = "math.meter";
      orthoSizeProps.value = camera->GetOrthoSize();
      orthoSizeProps.minValue = Camera::OrthoSizeMin();
      orthoSizeProps.maxValue = Camera::OrthoSizeMax();
      orthoSizeProps.speed = 0.1f;
      if (render.RenderDragFloat(orthoSizeProps)) {
        camera->SetOrthoSize(orthoSizeProps.value);
      }
    });
  }
}
}  // namespace mite