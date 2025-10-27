#ifndef MITE_PROPERTY_MATERIAL_H
#define MITE_PROPERTY_MATERIAL_H

#include "scene_core_components/material_component.h"
#include "ui_property.h"

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
  ColorEditProps m_BaseColorProps, m_EmissionProps;
  EditFloatProps m_MetallicProps, m_RoughnessProps, m_AmbientOcclusionProps;
};
}  // namespace mite

#endif  // MITE_PROPERTY_MATERIAL_H