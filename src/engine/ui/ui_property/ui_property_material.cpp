#include "ui_property_material.h"

namespace mite {
PropertyTable<MaterialComponent>::PropertyTable(MaterialComponent &component)
    : IPropertyTable("editor.material"), m_Component(component) {}

void PropertyTable<MaterialComponent>::Render(UIRender &render) {
  // 获取材质实例
  std::shared_ptr<MaterialInstance> instance =
      m_Component.GetMaterialInstance();
  if (!instance) return;

  // BaseColor基本色（图像显示/值编辑）
  RenderLabelItemRow(render, "editor.material_base_color_texture_using", [&]() {
    m_IsBaseColorTextureEnabled.checked = instance->IsBaseColorTextureEnabled();
    if (render.RenderCheckbox(m_IsBaseColorTextureEnabled)) {
      instance->SetBaseColorTextureEnabled(m_IsBaseColorTextureEnabled.checked);
    }
  });

  if (instance->IsBaseColorTextureEnabled()) {
    // 显示图像
    RenderLabelItemRow(render, "editor.material_base_color_texture", [&]() {
      m_BaseColorTexture.textureId =
          instance->GetBaseColorTexture().gpuHandle.apiHandle;
    });
  } else {
    // 编辑颜色
    RenderLabelItemRow(render, "editor.material_base_color", [&]() {
      m_BaseColorProps.color = instance->GetBaseColor();
      if (render.RenderColorEdit(m_BaseColorProps)) {
        instance->SetBaseColor(m_BaseColorProps.color);
      }
    });
  }

  // 金属度/粗糙度（图像显示/值编辑）
  RenderLabelItemRow(
      render, "editor.material_metallic_roughness_texture_using", [&]() {
        m_IsMetallicRoughnessTextureEnabled.checked =
            instance->IsMetallicRoughnessTextureEnabled();
        if (render.RenderCheckbox(m_IsMetallicRoughnessTextureEnabled)) {
          instance->SetMetallicRoughnessTextureEnabled(
              m_IsMetallicRoughnessTextureEnabled.checked);
        }
      });
  if (instance->IsMetallicRoughnessTextureEnabled()) {
    // 显示图像
    RenderLabelItemRow(
        render, "editor.material_metallic_roughness_texture", [&]() {
          m_BaseColorTexture.textureId =
              instance->GetMetallicRoughnessTexture().gpuHandle.apiHandle;
        });
  } else {
    // 编辑金属度/粗糙度
    RenderLabelItemRow(render, "editor.material_metallic", [&]() {
      m_MetallicProps.value = instance->GetMetallic();
      m_MetallicProps.maxValue = 1.0f;
      m_MetallicProps.minValue = 0.0f;
      if (render.RenderSliderFloat(m_MetallicProps)) {
        instance->SetMetallic(m_MetallicProps.value);
      }
    });
    RenderLabelItemRow(render, "editor.material_roughness", [&]() {
      m_RoughnessProps.value = instance->GetRoughness();
      m_RoughnessProps.maxValue = 1.0f;
      m_RoughnessProps.minValue = 0.0f;
      if (render.RenderSliderFloat(m_RoughnessProps)) {
        instance->SetRoughness(m_RoughnessProps.value);
      }
    });
  }

  // 环境光遮蔽
  RenderLabelItemRow(render, "editor.material_ambient_occlusion_texture_using",
                     [&]() {
                       m_IsOcclusionTextureEnabled.checked =
                           instance->IsOcclusionTextureEnabled();
                       if (render.RenderCheckbox(m_IsOcclusionTextureEnabled)) {
                         instance->SetOcclusionTextureEnabled(
                             m_IsOcclusionTextureEnabled.checked);
                       }
                     });
  if (instance->IsOcclusionTextureEnabled()) {
    // 显示图像
    RenderLabelItemRow(
        render, "editor.material_ambient_occlusion_texture", [&]() {
          m_BaseColorTexture.textureId =
              instance->GetOcclusionTexture().gpuHandle.apiHandle;
        });
  } else {
    // 编辑环境光遮蔽值
    RenderLabelItemRow(render, "editor.material_ambient_occlusion", [&]() {
      m_AmbientOcclusionProps.value = instance->GetAO();
      m_AmbientOcclusionProps.maxValue = 1.0f;
      m_AmbientOcclusionProps.minValue = 0.0f;
      if (render.RenderSliderFloat(m_AmbientOcclusionProps)) {
        instance->SetAO(m_AmbientOcclusionProps.value);
      }
    });
  }

  // 自发光
  RenderLabelItemRow(render, "editor.material_emission_texture_using", [&]() {
    m_IsEmissiveTextureEnabled.checked = instance->IsEmissiveTextureEnabled();
    if (render.RenderCheckbox(m_IsEmissiveTextureEnabled)) {
      instance->SetEmissiveTextureEnabled(m_IsEmissiveTextureEnabled.checked);
    }
  });
  if (instance->IsEmissiveTextureEnabled()) {
    // 显示图像
    RenderLabelItemRow(render, "editor.material_emission_texture", [&]() {
      m_BaseColorTexture.textureId =
          instance->GetEmissiveTexture().gpuHandle.apiHandle;
    });
  } else {
    // 编辑自发光颜色
    RenderLabelItemRow(render, "editor.material_emission", [&]() {
      m_EmissionProps.color = instance->GetEmission();
      if (render.RenderColorEdit(m_EmissionProps)) {
        instance->SetEmission(m_EmissionProps.color);
      }
    });
  }

  // 法线纹理
  RenderLabelItemRow(render, "editor.material_normal_texture_using", [&]() {
    m_IsNormalTextureEnabled.checked = instance->IsNormalTextureEnabled();
    if (render.RenderCheckbox(m_IsNormalTextureEnabled)) {
      instance->SetNormalTextureEnabled(m_IsNormalTextureEnabled.checked);
    }
  });
  if (instance->IsNormalTextureEnabled()) {
    // 显示图像
    RenderLabelItemRow(render, "editor.material_normal_texture", [&]() {
      m_BaseColorTexture.textureId =
          instance->GetNormalTexture().gpuHandle.apiHandle;
    });
  }
}
}  // namespace mite