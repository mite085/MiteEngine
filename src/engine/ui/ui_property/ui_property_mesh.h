#ifndef MITE_PROPERTY_MESH_H
#define MITE_PROPERTY_MESH_H

#include "ui_property.h"
#include "scene_core_components/mesh_component.h"

namespace mite {
/**
 * @brief LightComponent的特化实现
 */
template<> class PropertyTable<MeshComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(MeshComponent &component);
  void Render(UIRender &render);

 private:
  MeshComponent &m_Component;
};

}  // namespace mite

#endif  // MITE_PROPERTY_MESH_H