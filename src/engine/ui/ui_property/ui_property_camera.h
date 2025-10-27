#ifndef MITE_PROPERTY_CAMERA_H
#define MITE_PROPERTY_CAMERA_H

#include "scene_core_components/camera_component.h"
#include "ui_property.h"

namespace mite {
/**
 * @brief CameraComponent的特化实现
 */
template<> class PropertyTable<CameraComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(CameraComponent &component);
  void Render(UIRender &render);

 private:
  static const EnumComboBoxList<CameraProjectionType, 2> m_CameraTypeList;
  CameraComponent &m_Component;
  ComboboxProps m_CameraTypeProps;
  EditFloatProps m_FovProps;
};
}  // namespace mite

#endif  // MITE_PROPERTY_CAMERA_H
