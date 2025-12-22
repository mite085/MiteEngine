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

  // 绘制Mesh属性页（显示LOD以及顶点与索引数量）
  for (uint32_t i = 0; i < mesh->GetLODCount(); i++) {
    // LOD等级
    RenderLabelItemRow(render, "editor.mesh_lod_level", [&]() {
      IntEditProps lodLevelProps;
      lodLevelProps.value = i;
      render.RenderReadOnlyInt(lodLevelProps);
    });

    // 顶点计数
    RenderLabelItemRow(render, "editor.mesh_vertix_count", [&]() {
      IntEditProps meshVertixCount;
      meshVertixCount.value = mesh->GetVertexCount(i);
      render.RenderReadOnlyInt(meshVertixCount);
    });

    // 索引计数
    RenderLabelItemRow(render, "editor.mesh_index_count", [&]() {
      IntEditProps meshIndexCount;
      meshIndexCount.value = mesh->GetIndexCount(i);
      render.RenderReadOnlyInt(meshIndexCount);
    });

    // 绘制空行进行分割操作（最后一行不绘制）
    if (i == (mesh->GetLODCount() - 1)) {
      break;
    }
    RenderLabelItemRow(render, "", [&]() {});
  }
}
}  // namespace mite