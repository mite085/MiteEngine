#ifndef MITE_DATA_MODEL
#define MITE_DATA_MODEL

#include "mesh.h"

namespace mite {
/**
 * 完整模型封装（包含多个子网格）
 * 职责：
 * - 管理模型所有GPU资源
 * - 提供子网格访问接口
 */
class Model {
 public:
  /**
   * 构造函数
   * @param modelHandle 整个模型的GPU资源句柄
   */
  explicit Model(std::shared_ptr<ModelGPUHandle> modelHandle);

  /**
   * 获取子网格数量
   */
  size_t GetSubMeshCount() const
  {
    return subMeshes_.size();
  }

  /**
   * 获取指定子网格
   * @param index 子网格索引
   * @return 共享指针指向Mesh对象，无效索引返回nullptr
   */
  std::shared_ptr<Mesh> GetSubMesh(size_t index) const;

  /**
   * 获取模型级包围盒
   */
  const std::pair<glm::vec3, glm::vec3> &GetBoundingBox() const
  {
    return boundingBox_;
  }

 private:
  std::shared_ptr<ModelGPUHandle> modelHandle_;   // 整个模型的GPU资源
  std::vector<std::shared_ptr<Mesh>> subMeshes_;  // 子网格集合
  std::pair<glm::vec3, glm::vec3> boundingBox_;   // 模型级包围盒(min, max)
};
};  // namespace mite

#endif
