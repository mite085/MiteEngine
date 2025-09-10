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
  size_t GetSubMeshCount() const;

  /**
   * 获取指定子网格
   * @param index 子网格索引
   * @return 共享指针指向Mesh对象，无效索引返回nullptr
   */
  std::shared_ptr<Mesh> GetSubMesh(size_t index) const;
  /**
   * 获取所有子网格
   */
  const std::vector<std::shared_ptr<Mesh>> &GetAllSubMeshes() const;
  /**
   * 获取支持的LOD级别列表
   */
  std::vector<uint32_t> GetSupportedLODLevels() const;

  /**
   * 获取模型级包围盒
   */
  const std::pair<glm::vec3, glm::vec3> &GetBoundingBox() const;
  /**
   * 获取文件路径，用于调试
   */
  const std::string GetPath() const;
  /**
   * 检查模型是否包含LOD
   */
  bool HasLOD() const;

 private:
  std::shared_ptr<ModelGPUHandle> m_ModelHandle;   // 整个模型的GPU资源
  std::vector<std::shared_ptr<Mesh>> m_SubMeshes;  // 子网格集合
  std::pair<glm::vec3, glm::vec3> m_BoundingBox;   // 模型级包围盒(min, max)
  bool m_HasLOD = false;                           // 是否包含LOD
};
};  // namespace mite

#endif
