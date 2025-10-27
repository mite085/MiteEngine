#ifndef MITE_PROPERTY_BOUNDING_VOLUME_H
#define MITE_PROPERTY_BOUNDING_VOLUME_H

#include "ui_property.h"
#include "scene_core_components/bounding_volume_component.h"

namespace mite {
/**
 * @brief BoundingVolumeComponent的特化实现
 */
template<> class PropertyTable<BoundingVolumeComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(BoundingVolumeComponent &component);
  void Render(UIRender &render);

 private:
  void RenderAABBProperty(UIRender &render, const BoundingVolumeAABB &aabb);
  void RenderSphereProperty(UIRender &render, const BoundingVolumeSphere &sphere);
  void RenderOBBProperty(UIRender &render, const BoundingVolumeOBB &obb);
  void RenderPlaneProperty(UIRender &render, const BoundingVolumePlane &plane);

  static const EnumComboBoxList<BoundingVolumeType, 5> m_BoundingVolumeTypeList;
  BoundingVolumeComponent &m_Component;
  ComboboxProps m_VolumeTypeProps;
};

}  // namespace mite

#endif  // MITE_PROPERTY_BOUNDING_VOLUME_H