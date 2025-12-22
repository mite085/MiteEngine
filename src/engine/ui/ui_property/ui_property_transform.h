#ifndef MITE_PROPERTY_TRANSFORM_H
#define MITE_PROPERTY_TRANSFORM_H

#include "scene_core_components/transform_component.h"
#include "ui_property.h"

namespace mite {
/**
 * @brief TransformComponent的特化实现
 */
template <>
class PropertyTable<TransformComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(TransformComponent &component);

  void Render(UIRender &render);

 private:
  static const EnumComboBoxList<Transform::EulerOrder, 6> m_EulerOrderList;
  TransformComponent &m_Component;
  Float3EditProps m_PosProps, m_RotProps, m_SclProps;
  ComboboxProps m_RotTypeProps;
};
}  // namespace mite

#endif  // MITE_PROPERTY_TRANSFORM_H