#ifndef MITE_PROPERTIES_H
#define MITE_PROPERTIES_H

#include "scene_core_components/component_headers.h"
#include "scene_graph.h"
#include "ui_core/ui_render.h"
#include "ui_core/ui_enum_combobox.h"

namespace mite {
/**
 * @brief 属性基类 - 基于模板特化的自动映射
 */
template<typename T> class PropertyTable;

/**
 * @brief TransformComponent的特化实现
 */
template<> class PropertyTable<TransformComponent> {
 public:
  explicit PropertyTable(TransformComponent &component) : m_Component(component) {}

  void Render(UIRender &render);
 private:
  static const EnumComboBoxList<Transform::EulerOrder, 6> m_EulerOrderList;
  TransformComponent &m_Component;
};

/**
 * @brief CameraComponent的特化实现
 */
template<> class PropertyTable<CameraComponent> {
 public:
  explicit PropertyTable(CameraComponent &component) : m_Component(component) {}
  void Render(UIRender &render);

 private:
  static const EnumComboBoxList<CameraProjectionType, 2> m_CameraTypeList;
  CameraComponent &m_Component;
};

}  // namespace mite

#endif  // MITE_PROPERTIES_H
