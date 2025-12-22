#ifndef MITE_SPATIAL_PARTITION_H
#define MITE_SPATIAL_PARTITION_H

#include "basic_data/bounding_volume.h"
#include "basic_data/frustum.h"
#include "basic_data/ray.h"
#include "scene_node.h"

namespace mite {
/**
 * @class SpatialPartition
 * @brief 空间划分抽象接口，定义空间数据结构的统一接口
 *
 * 用于场景空间管理和快速查询，支持多种空间划分算法（BVH、四叉树、八叉树等）
 * 采用策略模式，允许运行时切换不同的空间划分实现
 */
class SpatialPartition {
 public:
  virtual ~SpatialPartition() = default;

  // ==================== 空间划分生命周期管理 ====================
  /**
   * @brief 更新场景节点在空间结构中的位置
   * @param node 要更新的场景节点
   */
  virtual bool Contains(std::shared_ptr<SceneNode> node) const = 0;
  /**
   * @brief 插入场景节点到空间结构中
   * @param node 要插入的场景节点
   */
  virtual void Insert(std::shared_ptr<SceneNode> node) = 0;
  /**
   * @brief 从空间结构中移除场景节点
   * @param node 要移除的场景节点
   */
  virtual void Remove(std::shared_ptr<SceneNode> node) = 0;
  /**
   * @brief 更新场景节点在空间结构中的位置
   * @param node 要更新的场景节点
   */
  virtual void Update(std::shared_ptr<SceneNode> node) = 0;
  /**
   * @brief 清空整个空间结构
   */
  virtual void Clear() = 0;
  /**
   * @brief 重建空间结构（优化性能）
   */
  virtual void Rebuild() = 0;

  // ==================== 空间结构外部查询接口 ====================
  /**
   * @brief 射线检测，返回所有相交的场景节点
   * @param ray 检测射线
   * @param results 相交结果列表（输出参数）
   * @return 是否找到相交节点
   */
  virtual bool Raycast(const Ray &ray,
                       std::vector<std::shared_ptr<SceneNode> > &results) = 0;
  /**
   * @brief 射线检测，返回第一个相交的场景节点
   * @param ray 检测射线
   * @param result 相交结果（输出参数）
   * @param distance 相交距离（输出参数）
   * @return 是否找到相交节点
   *
   * 作用场景：鼠标点击场景交互操作
   */
  virtual bool RaycastFirst(const Ray &ray, std::shared_ptr<SceneNode> &result,
                            float &distance) = 0;
  /**
   * @brief 视锥体裁剪，返回视锥体内的所有场景节点
   * @param frustum 视锥体
   * @param visibleMask 可见性掩码（用于分层渲染）
   * @param results 可见节点列表（输出参数）
   * @return 可见节点数量
   *
   * 作用场景：光栅化渲染视锥体剔除
   */
  virtual size_t FrustumCull(
      const Frustum &frustum, const uint32_t visibleMask,
      std::vector<std::shared_ptr<SceneNode> > &results) = 0;
  /**
   * @brief 通用包围体查询，返回包围体内的所有场景节点
   * @param volume 查询包围体
   * @param results 结果节点列表（输出参数）
   * @return 结果节点数量
   *
   * 作用场景：鼠标框选场景操作
   */
  virtual size_t VolumeQuery(
      const BoundingVolume &volume,
      std::vector<std::shared_ptr<SceneNode> > &results) = 0;
  /**
   * @brief 点查询，返回包含点的所有场景节点
   * @param point 查询点
   * @param results 结果节点列表（输出参数）
   * @return 结果节点数量
   */
  virtual size_t PointQuery(
      const glm::vec3 &point,
      std::vector<std::shared_ptr<SceneNode> > &results) = 0;
  /**
   * @brief 最近邻查询，返回距离点最近的场景节点
   * @param point 查询点
   * @param result 最近节点（输出参数）
   * @param maxDistance 最大搜索距离
   * @return 是否找到节点
   *
   * 作用场景：Runtime游戏交互操作
   */
  virtual bool NearestNeighbor(const glm::vec3 &point,
                               std::shared_ptr<SceneNode> &result,
                               float maxDistance = FLT_MAX) = 0;

  // ==================== 空间结构内部查询接口 ====================
  /**
   * @brief 遍历所有场景节点执行回调函数
   * @param callback 回调函数，返回false可中断遍历
   */
  virtual void ForEachNode(
      std::function<bool(std::shared_ptr<SceneNode>)> callback) = 0;
  /**
   * @brief 获取空间结构中节点的总数
   * @return 节点数量
   */
  virtual size_t GetNodeCount() const = 0;
  /**
   * @brief 判断空间结构是否为空
   * @return 是否为空
   */
  virtual bool IsEmpty() const = 0;
  /**
   * @brief 获取空间结构的深度（用于调试）
   * @return 结构深度
   */
  virtual int GetDepth() const = 0;
  /**
   * @brief 获取空间结构的类型名称
   * @return 类型名称字符串
   */
  virtual const char *GetTypeName() const = 0;
  /**
   * @brief 获取空间结构的性能统计信息
   * @return 统计信息字符串
   */
  virtual std::string GetStats() const = 0;
  /**
   * @brief 调试绘制接口（可选实现）
   * @param drawCallback 绘制回调函数
   */
  virtual void DebugDraw(
      std::function<void(const BoundingVolumeAABB &, int depth)>
          drawCallback) = 0;
};

/**
 * @brief 空间划分类型枚举
 */
enum class SpatialPartitionType {
  BVH,       // 包围盒层次结构
  QuadTree,  // 四叉树（2D空间）
  Octree,    // 八叉树（3D空间）
  Grid,      // 均匀网格
  KDTree     // KD树
};

/**
 * @brief 创建指定类型的空间划分实例
 * @param type 空间划分类型
 * @return 空间划分实例指针
 */
std::unique_ptr<SpatialPartition> CreateSpatialPartition(
    SpatialPartitionType type);

/**
 * @brief 获取空间划分类型的名称
 * @param type 空间划分类型
 * @return 类型名称字符串
 */
const char *GetSpatialPartitionTypeName(SpatialPartitionType type);
}  // namespace mite

#endif  // MITE_SPATIAL_PARTITION_H
