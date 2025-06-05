#include "renderer.h"

namespace mite {
VertexBuffer *VertexBuffer::Create(float *vertices, uint32_t size)
{
  return nullptr;
}
IndexBuffer *IndexBuffer::Create(uint32_t *indices, uint32_t count)
{
  return nullptr;
}
VertexArray *VertexArray::Create()
{
  return nullptr;
}
Shader *Shader::Create(const std::string &vsPath, const std::string &fsPath)
{
  return nullptr;
}
}  // namespace mite