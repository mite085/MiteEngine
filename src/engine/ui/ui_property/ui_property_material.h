#ifndef MITE_PROPERTY_MATERIAL_H
#define MITE_PROPERTY_MATERIAL_H

#include "ui_property.h"
#include "scene_core_components/material_component.h"

namespace mite {

/**
 * @brief MaterialComponent的特化实现
 */
template<> class PropertyTable<MaterialComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(MaterialComponent &component);
  void Render(UIRender &render);

 private:
  MaterialComponent &m_Component;
};

}  // namespace mite

#endif  // MITE_PROPERTY_MATERIAL_H