#include "ui_property_light.h"

namespace mite {
PropertyTable<LightComponent>::PropertyTable(LightComponent &component)
    : IPropertyTable("editor.light"), m_Component(component)
{
}

void PropertyTable<LightComponent>::Render(UIRender &render) {
    // 绘制光源属性页
}

}  // namespace mite