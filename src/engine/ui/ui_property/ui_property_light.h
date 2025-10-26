#ifndef MITE_PROPERTY_LIGHT_H
#define MITE_PROPERTY_LIGHT_H

#include "ui_property.h"
#include "scene_core_components/light_component.h"

namespace mite {
/**
 * @brief LightComponent的特化实现
 */
template<> class PropertyTable<LightComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(LightComponent &component);
  void Render(UIRender &render);

 private:
  LightComponent &m_Component;
};

}  // namespace mite

#endif  // MITE_PROPERTY_LIGHT_H