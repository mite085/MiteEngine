#ifndef MITE_DATA_MESH
#define MITE_DATA_MESH

#include "basic_type/handle_type.h"
#include "headers/headers.h"

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
   * @param baseSection 基础LOD级别的网格数据段信息
   * @param lodSections_ 所有LOD级别的网格数据段信息
   */
  Mesh(std::shared_ptr<ModelGPUHandle> modelHandle,
       const MeshSection &baseSection,
       const std::vector<MeshSection> &lodSections);

  /**
   * 获取指定LOD级别的顶点数量
   */
  uint32_t GetVertexCount(uint32_t lodLevel = 0) const;
  /**
   * 获取指定LOD级别的索引数量
   */
  uint32_t GetIndexCount(uint32_t lodLevel = 0) const;
  /**
   * 获取指定LOD级别的索引偏移
   */
  uint32_t GetIndexOffset(uint32_t lodLevel = 0) const;
  /**
   * 获取指定LOD级别的网格数据段信息
   */
  const MeshSection &GetSection(uint32_t lodLevel = 0) const;
  /**
   * 获取基础LOD级别的网格数据段信息
   */
  const MeshSection &GetBaseSection() const;
  /**
   * 获取所有LOD级别的网格数据段信息
   */
  const std::vector<MeshSection> &GetAllLODSections() const;
  /**
   * 获取支持的LOD级别数量
   */
  uint32_t GetLODCount() const;

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
  const std::pair<glm::vec3, glm::vec3> GetBoundingBox(uint32_t lodLevel = 0) const
  {
    return {section_.bboxMin, section_.bboxMax};
  }

 private:
  std::shared_ptr<ModelGPUHandle> modelHandle_;  // 父模型资源
  MeshSection baseSection_;                      // 基础LOD级别的网格数据段信息
  std::vector<MeshSection> lodSections_;         // 所有LOD级别的网格数据段信息
};
};  // namespace mite

#endif