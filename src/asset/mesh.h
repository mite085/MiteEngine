#ifndef MITE_RENDERER_MESH
#define MITE_RENDERER_MESH

#include "handle_types.h"

namespace mite {
/**
 * 子网格运行时封装
 * 职责：
 * - 维护单个子网格的GPU资源
 * - 提供最小化绘制接口
 */
class Mesh {
 public:
  Mesh(const MeshGPUHandle &handle);

  void Draw() const;
  uint32_t GetIndexCount() const
  {
    return handle_.indexCount;
  }
  MeshGPUHandle GetHandle() const
  {
    return handle_;
  }

 private:
  MeshGPUHandle handle_;
};

/**
 * 模型绘制事件
 * 职责：委托RendererDevice绘制Mesh
 */
class MeshDrawEvent : public Event {
 public:
  MeshDrawEvent(MeshGPUHandle mesh) : m_Mesh(mesh) {}

  MeshGPUHandle GetHandle()
  {
    return m_Mesh;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new MeshDrawEvent(m_Mesh);
  }

 private:
  MeshGPUHandle m_Mesh;
};
};  // namespace mite

#endif