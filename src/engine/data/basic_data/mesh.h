#ifndef MITE_DATA_MESH
#define MITE_DATA_MESH

#include "headers/headers.h"
#include "basic_type/handle_type.h"

namespace mite {
/**
 * 子网格封装
 * 职责：
 * - 引用父模型的GPU资源
 * - 提供子网格特定数据（偏移量、计数等）
 */
class Mesh {
 public:
  /**
   * 构造函数
   * @param modelHandle 父模型的GPU资源
   * @param section 子网格数据段信息
   */
  Mesh(std::shared_ptr<ModelGPUHandle> modelHandle, const MeshSection &section);

  /**
   * 获取顶点数量
   */
  uint32_t GetVertexCount() const
  {
    return section_.vertexCount;
  }

  /**
   * 获取索引数量
   */
  uint32_t GetIndexCount() const
  {
    return section_.indexCount;
  }
  /**
   * 获取索引偏移
   */
  uint32_t GetIndexOffset() const
  {
    return section_.indexOffset;
  }

  /**
   * 获取子网格数据段信息
   */
  const MeshSection &GetSection() const
  {
    return section_;
  }

  /**
   * 获取父模型GPU资源
   */
  std::shared_ptr<ModelGPUHandle> GetModelHandle() const
  {
    return modelHandle_;
  }

  /**
   * 获取子网格级包围盒
   */
  const std::pair<glm::vec3, glm::vec3> &GetBoundingBox() const
  {
    return {section_.bboxMin, section_.bboxMax};
  }

 private:
  std::shared_ptr<ModelGPUHandle> modelHandle_;  // 父模型资源
  MeshSection section_;                          // 子网格数据段信息
};

};  // namespace mite

#endif