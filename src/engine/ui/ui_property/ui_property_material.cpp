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

  RenderLabelItemRow(render, "editor.material_base_color", [&]() {
    ColorEditProps baseColorProps;
    baseColorProps.color = instance->GetBaseColor();
    if (render.RenderColorEdit(baseColorProps)) {
      instance->SetBaseColor(baseColorProps.color);
    }
  });

  RenderLabelItemRow(render, "editor.material_metallic", [&]() {
    DragFloatProps metallicProps;
    metallicProps.value = instance->GetMetallic();
    metallicProps.maxValue = 1.0f;
    metallicProps.minValue = 0.0f;
    if (render.RenderDragFloat(metallicProps)) {
      instance->SetMetallic(metallicProps.value);
    }
  });
  RenderLabelItemRow(render, "editor.material_roughness", [&]() {
    DragFloatProps roughnessProps;
    roughnessProps.value = instance->GetRoughness();
    roughnessProps.maxValue = 1.0f;
    roughnessProps.minValue = 0.0f;
    if (render.RenderDragFloat(roughnessProps)) {
      instance->SetRoughness(roughnessProps.value);
    }
  });
  RenderLabelItemRow(render, "editor.material_ambient_occlusion", [&]() {
    DragFloatProps ambientOcclusionProps;
    ambientOcclusionProps.value = instance->GetAO();
    ambientOcclusionProps.maxValue = 1.0f;
    ambientOcclusionProps.minValue = 0.0f;
    if (render.RenderDragFloat(ambientOcclusionProps)) {
      instance->SetAO(ambientOcclusionProps.value);
    }
  });
  RenderLabelItemRow(render, "editor.material_emission", [&]() {
    ColorEditProps emissionProps;
    emissionProps.color = instance->GetEmission();
    if (render.RenderColorEdit(emissionProps)) {
      instance->SetEmission(emissionProps.color);
    }
  });
}
}  // namespace mite