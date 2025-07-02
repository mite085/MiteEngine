#include "mesh.h"
#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后

namespace mite {
Mesh::Mesh(const MeshData &subMesh) : m_Layout(subMesh.layout)
{
  // 实际创建由IRenderDevice完成（见后文）
}

Mesh::~Mesh()
{
  // 析构时无需操作，由AssetManager统一管理生命周期
}

void Mesh::Draw() const
{
  //glBindVertexArray(static_cast<GLuint>(m_Handle.vertexArray));
  //glDrawElements(
  //    GL_TRIANGLES, static_cast<GLsizei>(m_Handle.indexCount), GL_UNSIGNED_INT, nullptr);
}

};  // namespace mite