#ifndef MITE_PROPERTY_VISIBILITY_H
#define MITE_PROPERTY_VISIBILITY_H

#include "ui_property.h"
#include "scene_core_components/visibility_component.h"

namespace mite {
/**
 * @brief LightComponent的特化实现
 */
template<> class PropertyTable<VisibilityComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(VisibilityComponent &component);
  void Render(UIRender &render);

 private:
  VisibilityComponent &m_Component;
};

}  // namespace mite

#endif  // MITE_PROPERTY_VISIBILITY_H