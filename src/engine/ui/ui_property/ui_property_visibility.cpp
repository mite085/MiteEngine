#include "ui_property_visibility.h"

namespace mite {
PropertyTable<VisibilityComponent>::PropertyTable(VisibilityComponent &component)
    : IPropertyTable("editor.visibility"), m_Component(component)
{
}

void PropertyTable<VisibilityComponent>::Render(UIRender &render)
{
  // 绘制可见性属性页
}

}  // namespace mite