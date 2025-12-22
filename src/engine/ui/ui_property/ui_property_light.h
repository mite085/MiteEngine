#ifndef MITE_PROPERTY_LIGHT_H
#define MITE_PROPERTY_LIGHT_H

#include "scene_core_components/light_component.h"
#include "ui_property.h"

namespace mite {
/**
 * @brief LightComponent的特化实现
 */
template <>
class PropertyTable<LightComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(LightComponent &component);
  void Render(UIRender &render);

 private:
  void RenderPointLightProperty(UIRender &render);
  void RenderSpotLightProperty(UIRender &render);
  void RenderDirectionalLightProperty(UIRender &render);
  void RenderAreaRectLightProperty(UIRender &render);
  void RenderAreaEllipseLightProperty(UIRender &render);

  static const EnumComboBoxList<LightType, 5> m_LightTypeList;
  LightComponent &m_Component;
};
}  // namespace mite

#endif  // MITE_PROPERTY_LIGHT_H