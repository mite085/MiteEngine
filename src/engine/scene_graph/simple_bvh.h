#ifndef MITE_SIMPLE_BVH_H
#define MITE_SIMPLE_BVH_H

#include "spatial_partition.h"

namespace mite {
/**
 * @class BVHNode
 * @brief BVH树节点，用于构建层次包围盒结构
 */
struct BVHNode {
  BoundingVolumeAABB bounds;  // 节点包围盒
  BVHNode *left = nullptr;    // 左子节点
  BVHNode *right = nullptr;   // 右子节点
  std::vector<std::shared_ptr<SceneNode>>
      sceneNodes;    // 关联的场景节点（叶子节点）
  size_t depth = 0;  // 节点深度

  /**
   * @brief 判断是否为叶子节点
   * @return 是否为叶子节点
   */
  bool IsLeaf() const {
    return !sceneNodes.empty();  // 有场景节点就是叶子节点
  }

  /**
   * @brief 获取节点高度（叶子节点为0）
   * @return 节点高度
   */
  size_t GetHeight() const;
};

/**
 * @class BVHStates
 * @brief BVH状态，用于递归统计节点信息
 */
struct BVHStats {
  size_t leafCount = 0;
  size_t internalCount = 0;
  size_t maxDepth = 0;
  size_t totalDepth = 0;
  float avgDepth = 0.0f;
};

/**
 * @class SimpleBVH
 * @brief 简易包围盒层次结构实现，基于SAH（Surface Area Heuristic）优化
 *
 * 采用自顶向下的构建方式，支持动态更新和高效的空间查询
 */
class SimpleBVH : public SpatialPartition {
 public:
  /**
   * @brief 构造函数
   * @param maxDepth 最大深度限制（防止过度细分）
   * @param minLeafSize 叶子节点最小包含对象数
   */
  explicit SimpleBVH(size_t maxDepth = 20, size_t minLeafSize = 4);

  /**
   * @brief 析构函数
   */
  ~SimpleBVH() override;

  // ==================== 空间划分生命周期管理 ====================
  bool Contains(std::shared_ptr<SceneNode> node) const override;
  void Insert(std::shared_ptr<SceneNode> node) override;
  void Remove(std::shared_ptr<SceneNode> node) override;
  /**
   * @brief 更新场景节点在空间结构中的位置
   * @param node
   *
   * 当SceneNode的Parent、Tranform或者BoundingBolume改变时，触发该函数
   */
  void Update(std::shared_ptr<SceneNode> node) override;
  void Clear() override;
  void Rebuild() override;

  // ==================== 空间结构外部查询接口 ====================
  bool Raycast(const Ray &ray,
               std::vector<std::shared_ptr<SceneNode>> &results) override;
  bool RaycastFirst(const Ray &ray, std::shared_ptr<SceneNode> &result,
                    float &distance) override;
  size_t FrustumCull(const Frustum &frustum, const uint32_t visibleMask,
                     std::vector<std::shared_ptr<SceneNode>> &results) override;
  size_t VolumeQuery(const BoundingVolume &volume,
                     std::vector<std::shared_ptr<SceneNode>> &results) override;
  size_t PointQuery(const glm::vec3 &point,
                    std::vector<std::shared_ptr<SceneNode>> &results) override;
  bool NearestNeighbor(const glm::vec3 &point,
                       std::shared_ptr<SceneNode> &result,
                       float maxDistance = FLT_MAX) override;

  // ==================== 空间结构内部查询接口 ====================
  void ForEachNode(
      std::function<bool(std::shared_ptr<SceneNode>)> callback) override;
  size_t GetNodeCount() const override;
  bool IsEmpty() const override;
  size_t GetDepth() const override;
  const char *GetTypeName() const override;
  std::string GetStats() const override;
  void DebugDraw(std::function<void(const BoundingVolumeAABB &, size_t depth)>
                     drawCallback) override;

 private:
  // ==================== 私有方法：BVH树构建 ====================
  /**
   * @brief 递归构建BVH树
   * @param nodes 场景节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param depth 当前深度
   * @return 构建的BVH节点
   */
  BVHNode *BuildTree(std::vector<std::shared_ptr<SceneNode>> &nodes,
                     size_t start, size_t end, size_t depth);
  /**
   * @brief 选择最佳分割轴和位置
   * @param nodes 节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param axis 最佳分割轴（输出参数）
   * @param splitPos 最佳分割位置（输出参数）
   * @return 是否找到有效分割
   */
  bool FindBestSplit(const std::vector<std::shared_ptr<SceneNode>> &nodes,
                     size_t start, size_t end, int &axis,
                     float &splitPos) const;
  /**
   * @brief 按指定轴和位置分割节点列表
   * @param nodes 节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param axis 分割轴
   * @param splitPos 分割位置
   * @return 分割点索引
   */
  size_t PartitionNodes(std::vector<std::shared_ptr<SceneNode>> &nodes,
                        size_t start, size_t end, int &axis,
                        float splitPos) const;
  /**
   * @brief 递归释放BVH节点
   * @param node 要释放的节点
   */
  void FreeNode(BVHNode *node);

  // ==================== 私有方法：BVH树增量更新 ====================
  /**
   * @brief 增量更新BVH树
   * @param forceFullRebuild 是否强制完全重建
   */
  void UpdateTree(bool forceFullRebuild = false);
  /**
   * @brief 递归更新节点及其子树
   * @param node 要更新的节点
   * @return 节点是否需要重构
   */
  bool UpdateNodeRecursive(BVHNode *node);
  /**
   * @brief 重构节点的包围盒
   * @param node 要重构的节点
   */
  void RefitNode(BVHNode *node);
  /**
   * @brief 检查节点是否需要重构
   * @param node 要检查的节点
   * @return 是否需要重构
   */
  bool NeedsRefit(BVHNode *node) const;
  /**
   * @brief
   * @param node
   */
  void FindDirtyBVHNodes(BVHNode *node);
  /**
   * @brief 标记节点为脏
   * @param node 脏节点
   */
  void MarkDirty(BVHNode *node);
  /**
   * @brief 清除脏标记
   */
  void ClearDirtyFlags();

  // ==================== 私有方法：BVH树递归查询 ====================
  /**
   * @brief 递归射线检测（使用最佳优先的遍历策略）
   * @param node 当前节点
   * @param ray 射线
   * @param results 结果列表
   *
   * 最佳优先遍历特点：
   * 1. 尽早找到最近交点，提前终止
   * 2. 减少不必要的子树遍历
   */
  void RaycastBestFirst(BVHNode *root, const Ray &ray,
                        std::vector<std::shared_ptr<SceneNode>> &results) const;
  /**
   * @brief 递归寻找第一个射线相交（使用最佳优先的遍历策略）
   * @param node 当前节点
   * @param ray 射线
   * @param bestNode 最佳节点（输出参数）
   * @param bestDistance 最佳距离（输出参数）
   */
  void RaycastFirstBestFirst(BVHNode *root, const Ray &ray,
                             std::shared_ptr<SceneNode> &bestNode,
                             float &bestDistance) const;
  /**
   * @brief 递归视锥体裁剪（使用广度优先的遍历策略）
   * @param node 当前节点
   * @param frustum 视锥体
   * @param results 结果列表
   *
   * 广度优先遍历特点：
   * 1. 缓存友好（连续内存访问）
   * 2. 适合处理与批量查询
   */
  void FrustumCullBFS(BVHNode *node, const Frustum &frustum,
                      const uint32_t visibleMask,
                      std::vector<std::shared_ptr<SceneNode>> &results) const;
  /**
   * @brief 递归包围盒查询（使用广度优先的遍历策略）
   * @param node 当前节点
   * @param sphere 球体
   * @param results 包围盒内所有SceneNode的结果列表
   */
  void VolumeQueryBFS(BVHNode *node, const BoundingVolume &sphere,
                      std::vector<std::shared_ptr<SceneNode>> &results) const;
  /**
   * @brief 递归遍历所有节点（深度优先遍历）
   * @param node 当前节点
   * @param callback 回调函数
   * @return 是否继续遍历
   */
  bool TraverseDFS(
      BVHNode *node,
      std::function<bool(std::shared_ptr<SceneNode>)> callback) const;
  /**
   * @brief 递归调试绘制
   * @param node 当前节点
   * @param drawCallback 绘制回调
   */
  void DebugDrawRecursive(
      BVHNode *node,
      std::function<void(const BoundingVolumeAABB &, size_t depth)>
          drawCallback) const;
  /**
   * @brief 递归统计节点信息
   * @param node 当前节点
   * @param stats 统计信息
   */
  void CollectStatsRecursive(BVHNode *node, struct BVHStats &stats) const;
  /**
   * @brief 计算射线与AABB的相交距离
   */
  float CalculateRayAABBDistance(const Ray &ray,
                                 const BoundingVolumeAABB &aabb) const;

  /**
   * @brief 计算点到AABB的最近距离平方
   */
  float CalculatePointAABBDistanceSq(const glm::vec3 &point,
                                     const BoundingVolumeAABB &aabb) const;

  // ==================== 私有方法：结构变化处理 ====================
  /**
   * @brief 判断是否需要完全重建.
   */
  bool ShouldRebuildCompletely() const;
  /**
   * @brief 处理结构变化（新增/删除节点）
   */
  void ProcessStructuralChanges();
  /**
   * @brief 批量插入节点
   */
  bool BatchInsertNodes(
      const std::unordered_set<std::shared_ptr<SceneNode>> &nodes);
  /**
   * @brief 移除空叶子节点
   */
  void RemoveEmptyLeaves();
  /**
   * @brief 尝试增量插入
   */
  bool TryInsertIntoExistingTree(std::shared_ptr<SceneNode> node);
  /**
   * @brief 构建SceneNode-BVHNode映射
   */
  void BuildNodeMapping(BVHNode *node);

 private:
  // BVH基本属性
  BVHNode *m_Root = nullptr;  // BVH根节点
  std::unordered_set<std::shared_ptr<SceneNode>>
      m_AllNodes;               // 所有场景节点列表（用于快速重建）
  size_t m_MaxDepth;            // 最大构建深度
  size_t m_MinLeafSize;         // 叶子节点最小对象数
  bool m_ForceRebuild = false;  // 强制重建标志
  size_t m_NodeCount = 0;       // 总节点数统计

  // BVH增量更新
  std::unordered_set<std::shared_ptr<SceneNode>> m_DirtyNodes;  // 脏节点集合
  std::unordered_set<BVHNode *> m_DirtyBVHNodes;                // 脏BVH节点集合

  // 结构变化跟踪
  std::unordered_map<std::shared_ptr<SceneNode>, BVHNode *>
      m_NodeToBVHNodeMap;  // 节点映射
  std::unordered_set<std::shared_ptr<SceneNode>>
      m_NewNodes;                               // 待插入的新增节点
  std::unordered_set<BVHNode *> m_EmptyLeaves;  // 空的叶子节点（待清理）

  // 性能统计
  mutable uint64_t m_RaycastTests = 0;  // 射线检测测试次数
  mutable uint64_t m_FrustumTests = 0;  // 视锥体测试次数
};
}  // namespace mite

#endif  // MITE_SIMPLE_BVH_H
