#ifndef MITE_PROPERTY_MATERIAL_H
#define MITE_PROPERTY_MATERIAL_H

#include "scene_core_components/material_component.h"
#include "ui_property.h"

namespace mite {
/**
 * @brief MaterialComponent的特化实现
 */
template <>
class PropertyTable<MaterialComponent> : public IPropertyTable {
 public:
  explicit PropertyTable(MaterialComponent &component);
  void Render(UIRender &render);

 private:
  MaterialComponent &m_Component;

  // 纹理启用flag
  CheckboxProps m_IsBaseColorTextureEnabled,
      m_IsMetallicRoughnessTextureEnabled, m_IsOcclusionTextureEnabled,
      m_IsEmissiveTextureEnabled, m_IsNormalTextureEnabled;

  // 纹理属性
  ImageProps m_BaseColorTexture, m_MetallicRoughnessTexture, m_OcclusionTexture,
      m_EmissiveTexture, m_NormalTexture;

  // 非纹理状态下的值属性
  ColorEditProps m_BaseColorProps, m_EmissionProps;
  FloatEditProps m_MetallicProps, m_RoughnessProps, m_AmbientOcclusionProps;
};
}  // namespace mite

#endif  // MITE_PROPERTY_MATERIAL_H