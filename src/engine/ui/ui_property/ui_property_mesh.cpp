#include "ui_property_mesh.h"

namespace mite {
PropertyTable<MeshComponent>::PropertyTable(MeshComponent &component)
    : IPropertyTable("editor.mesh"), m_Component(component)
{
}

void PropertyTable<MeshComponent>::Render(UIRender &render)
{
  // 获取mesh
  std::shared_ptr<Mesh> mesh = m_Component.GetMesh();
  if (!mesh)
    return;

  // 绘制Mesh属性页

  // LOD级别选择


}
}  // namespace mite