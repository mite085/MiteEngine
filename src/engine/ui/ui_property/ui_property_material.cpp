#include "ui_property_material.h"

namespace mite {
PropertyTable<MaterialComponent>::PropertyTable(MaterialComponent &component)
    : IPropertyTable("editor.material"), m_Component(component)
{
}

void PropertyTable<MaterialComponent>::Render(UIRender &render)
{
  // 获取材质实例
  std::shared_ptr<MaterialInstance> instance = m_Component.GetMaterialInstance();
  if (!instance)
    return;

  // BaseColor基本色
  RenderLabelItemRow(render, "editor.material_base_color", [&]() {
    m_BaseColorProps.color = instance->GetBaseColor();
    if (render.RenderColorEdit(m_BaseColorProps)) {
      instance->SetBaseColor(m_BaseColorProps.color);
    }
  });

  // 金属度
  RenderLabelItemRow(render, "editor.material_metallic", [&]() {
    m_MetallicProps.value = instance->GetMetallic();
    m_MetallicProps.maxValue = 1.0f;
    m_MetallicProps.minValue = 0.0f;
    if (render.RenderSliderFloat(m_MetallicProps)) {
      instance->SetMetallic(m_MetallicProps.value);
    }
  });

  // 粗糙度
  RenderLabelItemRow(render, "editor.material_roughness", [&]() {
    m_RoughnessProps.value = instance->GetRoughness();
    m_RoughnessProps.maxValue = 1.0f;
    m_RoughnessProps.minValue = 0.0f;
    if (render.RenderSliderFloat(m_RoughnessProps)) {
      instance->SetRoughness(m_RoughnessProps.value);
    }
  });

  // 环境光遮蔽
  RenderLabelItemRow(render, "editor.material_ambient_occlusion", [&]() {
    m_AmbientOcclusionProps.value = instance->GetAO();
    m_AmbientOcclusionProps.maxValue = 1.0f;
    m_AmbientOcclusionProps.minValue = 0.0f;
    if (render.RenderSliderFloat(m_AmbientOcclusionProps)) {
      instance->SetAO(m_AmbientOcclusionProps.value);
    }
  });

  // 自发光
  RenderLabelItemRow(render, "editor.material_emission", [&]() {
    m_EmissionProps.color = instance->GetEmission();
    if (render.RenderColorEdit(m_EmissionProps)) {
      instance->SetEmission(m_EmissionProps.color);
    }
  });
}
}  // namespace mite