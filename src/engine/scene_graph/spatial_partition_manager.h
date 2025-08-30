#ifndef MITE_SCENE_SPTIAL_PARTITION_MANAGER_H
#define MITE_SCENE_SPTIAL_PARTITION_MANAGER_H

#include "spatial_partition.h"

namespace mite {
// 前向声明
class SceneRegistry;

/**
 * @class SpatialPartitionManager
 * @brief 负责空间划分结构和空间查询
 */
class SpatialPartitionManager {
 public:
  explicit SpatialPartitionManager(
      SpatialPartitionType spatialPartitionType = SpatialPartitionType::BVH);
  ~SpatialPartitionManager() = default;

  void Clear();

  // ==================== 节点增删管理接口 ====================
  /**
   * @brief 添加节点到空间划分结构
   */
  void AddNodeToSpatialPartition(SceneNode *node);

  /**
   * @brief 更新场景节点在空间结构中的位置
   * @param node 要更新的场景节点
   */
  void Update(SceneNode *node);

  /**
   * @brief 从空间划分结构中移除节点
   */
  void RemoveNodeFromSpatialPartition(SceneNode *node);

  // ==================== 空间划分管理接口 ====================
  /**
   * @brief 设置空间划分类型
   * @param type 空间划分类型
   */
  void SetSpatialPartitionType(SpatialPartitionType type);

  /**
   * @brief 获取当前空间划分类型
   * @return 空间划分类型
   */
  SpatialPartitionType GetSpatialPartitionType() const;

  /**
   * @brief 重新构建空间划分结构（优化性能）
   */
  void RebuildSpatialPartition(std::vector<SceneNode *> nodelist);

  /**
   * @brief 获取空间划分统计信息
   * @return 统计信息字符串
   */
  std::string GetSpatialPartitionStats() const;

  /**
   * @brief 调试绘制接口
   * @param drawCallback 绘制回调函数
   */
  void DebugDraw(std::function<void(const AABB &, int depth)> drawCallback);

  // ==================== 空间查询接口（为SceneView提供优化） ====================
  /**
   * @brief 快速可见性检查（不返回具体节点，只计数）
   * @return 可见节点数量
   */
  size_t QueryVisibleCount(SceneRegistry &registry,
                           const Frustum &frustum,
                           uint32_t visibilityMask);

  /**
   * @brief 获取可见节点数量（不执行可见性检查，只获取上次检查结果）
   * @return 可见节点数量
   */
  size_t GetVisibleNodeCount() const;

  /**
   * @brief 视锥体裁剪查询 - 主要给SceneView使用
   * @param frustum 视锥体
   * @return 可见节点列表
   */
  std::vector<SceneNode *> QueryVisibleNodes(SceneRegistry &registry,
                                             const Frustum &frustum,
                                             uint32_t visibilityMask);

  /**
   * @brief 射线检测查询
   * @param ray 检测射线
   * @return 相交节点列表
   */
  std::vector<SceneNode *> QueryRaycast(SceneRegistry &registry,
                                        const Ray &ray,
                                        uint32_t visibilityMask);

  /**
   * @brief 射线检测查询（第一个命中）
   * @param ray 检测射线
   * @param result 命中的节点（输出参数）
   * @param distance 相交距离（输出参数）
   * @return 是否命中
   */
  bool QueryRaycastFirst(SceneRegistry &registry,
                         const Ray &ray,
                         SceneNode *&result,
                         float &distance,
                         uint32_t visibilityMask);

  /**
   * @brief 球体查询
   * @param sphere 查询球体
   * @return 结果节点列表
   */
  std::vector<SceneNode *> QuerySphere(SceneRegistry &registry,
                                       const Sphere &sphere,
                                       uint32_t visibilityMask);

  /**
   * @brief AABB查询
   * @param aabb 查询AABB
   * @return 结果节点列表
   */
  std::vector<SceneNode *> QueryAABB(SceneRegistry &registry,
                                     const AABB &aabb,
                                     uint32_t visibilityMask);

 private:
  // ==================== 内部工具方法 ====================
  /**
   * @brief 初始化空间划分结构
   */
  void InitializeSpatialPartition();

  // 空间划分结构
  std::unique_ptr<SpatialPartition> m_spatialPartition;

  // 当前空间划分类型
  SpatialPartitionType m_spatialPartitionType;

  // 可见节点数量存储
  size_t m_visibleNodeCount;

  // 线程安全保护
  mutable std::mutex m_mutex;

  // 日志器
  Logger m_logger;
};
}  // namespace mite

#endif  // MITE_SCENE_SPTIAL_PARTITION_MANAGER_H
