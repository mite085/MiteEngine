#include "ui_property_visibility.h"

namespace mite {
const EnumComboBoxList<uint32_t, 2>
    PropertyTable<VisibilityComponent>::m_VisibilityMaskList =
        EnumComboBoxList(std::array{
            std::pair{VisibilityMask::NONE, "editor.visibility_mask_none"},
            std::pair{VisibilityMask::ALL, "editor.visibility_mask_all"}});

PropertyTable<VisibilityComponent>::PropertyTable(
    VisibilityComponent &component)
    : IPropertyTable("editor.visibility"), m_Component(component) {}

void PropertyTable<VisibilityComponent>::Render(UIRender &render) {
  // 绘制可见性属性页
  RenderLabelItemRow(render, "editor.visibility_is_visible", [&]() {
    m_VisibilityProps.checked = m_Component.IsVisible();
    if (render.RenderCheckbox(m_VisibilityProps)) {
      m_Component.SetVisible(m_VisibilityProps.checked);
    }
  });

  // 绘制可见性掩码
  RenderLabelItemRow(render, "editor.visibility_mask", [&]() {
    m_VisibilityMaskProps.itemTranslationKeys =
        m_VisibilityMaskList.GetTranslateKeyList();  // 获取所有Keys
    m_VisibilityMaskProps.selectedIndex = m_VisibilityMaskList.GetIndex(
        m_Component.GetVisibilityMask());  // 获取当前index
    if (render.RenderCombobox(m_VisibilityMaskProps)) {
      m_Component.SetVisibilityMask(m_VisibilityMaskList.GetEnumType(
          m_VisibilityMaskProps
              .selectedIndex));  // 使用选择后的Index执行Set逻辑
    }
  });
}
}  // namespace mite