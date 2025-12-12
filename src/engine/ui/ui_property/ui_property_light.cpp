#include "ui_property_light.h"
#include "light_data/area_light.h"
#include "light_data/directional_light.h"
#include "light_data/point_light.h"
#include "light_data/spot_light.h"

namespace mite {
const EnumComboBoxList<LightType, 5> PropertyTable<LightComponent>::m_LightTypeList =
    EnumComboBoxList(std::array{std::pair{LightType::POINT, "editor.light_point"},
                                std::pair{LightType::SPOT, "editor.light_spot"},
                                std::pair{LightType::DIRECTIONAL, "editor.light_directional"},
                                std::pair{LightType::AREA_RECT, "editor.light_area_rect"},
                                std::pair{LightType::AREA_ELLIPSE, "editor.light_area_ellipse"}});

PropertyTable<LightComponent>::PropertyTable(LightComponent &component)
    : IPropertyTable("editor.light"), m_Component(component)
{
}

void PropertyTable<LightComponent>::Render(UIRender &render)
{
  // 光源类型显示（不支持更改）
  RenderLabelItemRow(render, "editor.light_type", [&]() {
    render.RenderLabel(m_LightTypeList.GetTranslateKey(m_Component.GetLightType()));
  });

  // 绘制详细属性页
  switch (m_Component.GetLightType()) {
    case LightType::POINT:
      RenderPointLightProperty(render);
      break;
    case LightType::SPOT:
      RenderSpotLightProperty(render);
      break;
    case LightType::DIRECTIONAL:
      RenderDirectionalLightProperty(render);
      break;
    case LightType::AREA_RECT:
      RenderAreaRectLightProperty(render);
      break;
    case LightType::AREA_ELLIPSE:
      RenderAreaEllipseLightProperty(render);
      break;
    default:
      break;
  }
}
void PropertyTable<LightComponent>::RenderPointLightProperty(UIRender &render)
{
  // 运行时动态检测
  std::shared_ptr<PointLight> pointLight = std::dynamic_pointer_cast<PointLight>(
      m_Component.GetLight());

  // 确保是点光源对象
  if (pointLight) {
    // 是否启用
    CheckboxProps m_PointEnableProps;
    m_PointEnableProps.checked = pointLight->IsEnabled();
    RenderLabelItemRow(render, "editor.light_enable", [&]() {
      if (render.RenderCheckbox(m_PointEnableProps)) {
        pointLight->SetEnabled(m_PointEnableProps.checked);
        ;
      }
    });

    // 颜色编辑
    ColorEditProps m_PointColorProps;
    m_PointColorProps.showAlpha = false;
    m_PointColorProps.color = {pointLight->GetColor(), 1.0f};
    RenderLabelItemRow(render, "editor.light_color", [&]() {
      if (render.RenderColorEdit(m_PointColorProps)) {
        pointLight->SetColor(glm::vec3(
            m_PointColorProps.color.x, m_PointColorProps.color.y, m_PointColorProps.color.z));
      }
    });

    // 强度编辑
    FloatEditProps m_PointLightIntensityProps;
    m_PointLightIntensityProps.value = pointLight->GetIntensity();
    m_PointLightIntensityProps.minValue = 0.0f;  // 最小值限制在0.0
    RenderLabelItemRow(render, "editor.light_intensity", [&]() {
      if (render.RenderDragFloat(m_PointLightIntensityProps)) {
        pointLight->SetIntensity(m_PointLightIntensityProps.value);
      }
    });

    // 影响半径编辑
    FloatEditProps m_PointLightRadiusProps;
    m_PointLightRadiusProps.value = pointLight->GetRadius();
    m_PointLightRadiusProps.minValue = 0.0f;  // 最小值限制在0.0
    RenderLabelItemRow(render, "editor.light_point_radius", [&]() {
      if (render.RenderDragFloat(m_PointLightRadiusProps)) {
        pointLight->SetRadius(m_PointLightRadiusProps.value);
      }
    });
    // 衰减系数编辑
    FloatEditProps m_PointLightFallOffProps;
    m_PointLightFallOffProps.value = pointLight->GetFalloff();
    m_PointLightFallOffProps.minValue = 0.0f;  // 最小值限制在0.0
    RenderLabelItemRow(render, "editor.light_point_falloff", [&]() {
      if (render.RenderDragFloat(m_PointLightFallOffProps)) {
        pointLight->SetFalloff(m_PointLightFallOffProps.value);
      }
    });
  }
}

void PropertyTable<LightComponent>::RenderDirectionalLightProperty(UIRender &render)
{  // 运行时动态检测
  std::shared_ptr<DirectionalLight> directionalLight = std::dynamic_pointer_cast<DirectionalLight>(
      m_Component.GetLight());
  // 确保是方向光对象
  if (directionalLight) {
    // 是否启用
    CheckboxProps m_DirectionalEnableProps;
    m_DirectionalEnableProps.checked = directionalLight->IsEnabled();
    RenderLabelItemRow(render, "editor.light_enable", [&]() {
      if (render.RenderCheckbox(m_DirectionalEnableProps)) {
        directionalLight->SetEnabled(m_DirectionalEnableProps.checked);
      }
    });
    // 颜色编辑
    ColorEditProps m_DirectionalColorProps;
    m_DirectionalColorProps.showAlpha = false;
    m_DirectionalColorProps.color = {directionalLight->GetColor(), 1.0f};
    RenderLabelItemRow(render, "editor.light_color", [&]() {
      if (render.RenderColorEdit(m_DirectionalColorProps)) {
        directionalLight->SetColor(glm::vec3(m_DirectionalColorProps.color.x,
                                             m_DirectionalColorProps.color.y,
                                             m_DirectionalColorProps.color.z));
      }
    });
    // 强度编辑
    FloatEditProps m_DirectionalLightIntensityProps;
    m_DirectionalLightIntensityProps.value = directionalLight->GetIntensity();
    m_DirectionalLightIntensityProps.minValue = 0.0f;  // 最小值限制在0.0
    RenderLabelItemRow(render, "editor.light_intensity", [&]() {
      if (render.RenderDragFloat(m_DirectionalLightIntensityProps)) {
        directionalLight->SetIntensity(m_DirectionalLightIntensityProps.value);
      }
    });
    // 辐照度编辑（方向光特有属性）（暂未启用，仅使用最简单的强度控制，待后续考虑物理量）
    //FloatEditProps m_DirectionalIrradiusProps;
    //m_DirectionalIrradiusProps.value = directionalLight->GetIrradius();
    //m_DirectionalIrradiusProps.minValue = 0.0f;  // 最小值限制在0.0
    //RenderLabelItemRow(render, "editor.light_directional_irradius", [&]() {
    //  if (render.RenderDragFloat(m_DirectionalIrradiusProps)) {
    //    directionalLight->SetIrradius(m_DirectionalIrradiusProps.value);
    //  }
    //});
  }
}
  void PropertyTable<LightComponent>::RenderSpotLightProperty(UIRender & render) {}
  void PropertyTable<LightComponent>::RenderAreaRectLightProperty(UIRender & render) {}
  void PropertyTable<LightComponent>::RenderAreaEllipseLightProperty(UIRender & render) {}
}  // namespace mite