#include "ui_property_bounding_volume.h"

namespace mite {
const EnumComboBoxList<BoundingVolumeType, 4>
    PropertyTable<BoundingVolumeComponent>::m_BoundingVolumeTypeList = EnumComboBoxList(
        std::array{std::pair{BoundingVolumeType::AABB, "editor.bounding_volume_aabb"},
                   std::pair{BoundingVolumeType::OBB, "editor.bounding_volume_obb"},
                   std::pair{BoundingVolumeType::Sphere, "editor.bounding_volume_sphere"},
                   std::pair{BoundingVolumeType::Plane, "editor.bounding_volume_plane"}});

PropertyTable<BoundingVolumeComponent>::PropertyTable(BoundingVolumeComponent &component)
    : IPropertyTable("editor.bounding_volume"), m_Component(component)
{
}

void PropertyTable<BoundingVolumeComponent>::Render(UIRender &render)
{
  // 绘制包围盒属性页

  // 局部包围盒类型
  RenderLabelItemRow(render, "editor.bounding_volume_type", [&]() {
    m_VolumeTypeProps.itemTranslationKeys =
        m_BoundingVolumeTypeList.GetTranslateKeyList();  // 获取所有Keys
    m_VolumeTypeProps.selectedIndex = m_BoundingVolumeTypeList.GetIndex(
        m_Component.GetVolume()->GetType());  // 获取当前index
    if (render.RenderCombobox(m_VolumeTypeProps)) {
      // 响应类型变换
      m_Component.GetVolume()->SetType(
          m_BoundingVolumeTypeList.GetEnumType(m_VolumeTypeProps.selectedIndex));
    }
  });

  // 根据类型绘制
  if (m_Component.GetVolume()->GetType() == BoundingVolumeType::AABB) {
    RenderAABBProperty(render, m_Component.GetVolume()->GetAABB());
  }
  else if (m_Component.GetVolume()->GetType() == BoundingVolumeType::Sphere) {
    RenderSphereProperty(render, m_Component.GetVolume()->GetSphere());
  }
  else if (m_Component.GetVolume()->GetType() == BoundingVolumeType::OBB) {
    RenderOBBProperty(render, m_Component.GetVolume()->GetOBB());
  }
  else if (m_Component.GetVolume()->GetType() == BoundingVolumeType::Plane) {
    RenderPlaneProperty(render, m_Component.GetVolume()->GetPlane());
  }
}
void PropertyTable<BoundingVolumeComponent>::RenderAABBProperty(UIRender &render,
                                                                const BoundingVolumeAABB &aabb)
{
  // 由于存在多种情况，不适合将属性放在成员变量中，所以作为临时变量
  // 轴对齐包围盒上下限
  EditFloat3Props aabbMaxProps, aabbMinProps;
  aabbMaxProps.translationKey = "math.meter";
  aabbMaxProps.value = aabb.max;
  aabbMinProps.translationKey = "math.meter";
  aabbMinProps.value = aabb.min;

  RenderLabelItemRow(render, "editor.bounding_volume_aabb_max", [&]() {
    render.RenderReadOnlyFloat3(aabbMaxProps);
  });
  RenderLabelItemRow(render, "editor.bounding_volume_aabb_min", [&]() {
    render.RenderReadOnlyFloat3(aabbMinProps);
  });
}

void PropertyTable<BoundingVolumeComponent>::RenderSphereProperty(
    UIRender &render, const BoundingVolumeSphere &sphere)
{
  // 球包围盒中心与半径
  EditFloat3Props centerProps;
  EditFloatProps radiusProps;
  centerProps.translationKey = "math.meter";
  centerProps.value = sphere.center;
  radiusProps.translationKey = "math.meter";
  radiusProps.value = sphere.radius;

  RenderLabelItemRow(render, "editor.bounding_volume_sphere_center", [&]() {
    render.RenderReadOnlyFloat3(centerProps);
  });
  RenderLabelItemRow(render, "editor.bounding_volume_sphere_radius", [&]() {
    render.RenderReadOnlyFloat(radiusProps);
  });
}

void PropertyTable<BoundingVolumeComponent>::RenderOBBProperty(UIRender &render,
                                                               const BoundingVolumeOBB &obb)
{
  // 轴对齐包围盒中心、半长和方向矩阵
  EditFloat3Props obbCenterProps, obbExtentsProps;
  obbCenterProps.translationKey = "math.meter";
  obbCenterProps.value = obb.center;
  obbExtentsProps.translationKey = "math.meter";
  obbExtentsProps.value = obb.extents;

  // TODO: 方向矩阵显示/转换为欧拉角

  RenderLabelItemRow(render, "editor.bounding_volume_obb_center", [&]() {
    render.RenderReadOnlyFloat3(obbCenterProps);
  });
  RenderLabelItemRow(render, "editor.bounding_volume_obb_extents", [&]() {
    render.RenderReadOnlyFloat3(obbExtentsProps);
  });
  RenderLabelItemRow(render, "editor.bounding_volume_obb_orientation", [&]() {
    render.RenderLabel("Invalid Orientation");
  });

}

void PropertyTable<BoundingVolumeComponent>::RenderPlaneProperty(UIRender &render,
                                                                 const BoundingVolumePlane &plane)
{
  // 球包围盒中心与半径
  EditFloat3Props normalProps;
  EditFloatProps distanceProps;
  normalProps.value = plane.normal;
  distanceProps.translationKey = "math.meter";
  distanceProps.value = plane.distance;

  RenderLabelItemRow(render, "editor.bounding_volume_plane_center", [&]() {
    render.RenderReadOnlyFloat3(normalProps);
  });
  RenderLabelItemRow(render, "editor.bounding_volume_plane_distance", [&]() {
    render.RenderReadOnlyFloat(distanceProps);
  });
}

}  // namespace mite