#ifndef MITE_PROPERTY_VISIBILITY_H
#define MITE_PROPERTY_VISIBILITY_H

#include "scene_core_components/visibility_component.h"
#include "ui_property.h"

namespace mite {
/**
 * @brief LightComponent的特化实现
 */
template <>
class PropertyTable<VisibilityComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(VisibilityComponent &component);
  void Render(UIRender &render);

 private:
  static const EnumComboBoxList<uint32_t, 2> m_VisibilityMaskList;
  VisibilityComponent &m_Component;
  CheckboxProps m_VisibilityProps;
  ComboboxProps m_VisibilityMaskProps;
};
}  // namespace mite

#endif  // MITE_PROPERTY_VISIBILITY_H