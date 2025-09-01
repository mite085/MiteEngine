#ifndef MITE_SIMPLE_BVH_H
#define MITE_SIMPLE_BVH_H

#include "spatial_partition.h"

namespace mite {
/**
 * @class BVHNode
 * @brief BVH树节点，用于构建层次包围盒结构
 */
struct BVHNode {
  AABB bounds;                          ///< 节点包围盒
  BVHNode *left = nullptr;              ///< 左子节点
  BVHNode *right = nullptr;             ///< 右子节点
  std::vector<SceneNode *> sceneNodes;  ///< 关联的场景节点（叶子节点）
  int depth = 0;                        ///< 节点深度

  /**
   * @brief 判断是否为叶子节点
   * @return 是否为叶子节点
   */
  bool IsLeaf() const
  {
    return !sceneNodes.empty();  // 有场景节点就是叶子节点
  }

  /**
   * @brief 获取节点高度（叶子节点为0）
   * @return 节点高度
   */
  int GetHeight() const;
};

/**
 * @class BVHStates
 * @brief BVH状态，用于递归统计节点信息
 */
struct BVHStats {
  int leafCount = 0;
  int internalCount = 0;
  int maxDepth = 0;
  int totalDepth = 0;
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
  explicit SimpleBVH(int maxDepth = 20, int minLeafSize = 4);

  /**
   * @brief 析构函数
   */
  ~SimpleBVH() override;

  // SpatialPartition接口实现
  void Insert(SceneNode *node) override;
  void Remove(SceneNode *node) override;
  void Update(SceneNode *node) override;
  void Clear() override;
  void Rebuild() override;

  bool Raycast(const Ray &ray, std::vector<SceneNode *> &results) override;
  bool RaycastFirst(const Ray &ray, SceneNode *&result, float &distance) override;
  int FrustumCull(const Frustum &frustum, std::vector<SceneNode *> &results) override;
  size_t SphereQuery(const Sphere &sphere, std::vector<SceneNode *> &results) override;
  size_t AABBQuery(const AABB &aabb, std::vector<SceneNode *> &results) override;
  size_t PointQuery(const glm::vec3 &point, std::vector<SceneNode *> &results) override;
  bool NearestNeighbor(const glm::vec3 &point,
                       SceneNode *&result,
                       float maxDistance = FLT_MAX) override;

  void ForEachNode(std::function<bool(SceneNode *)> callback) override;
  size_t GetNodeCount() const override;
  bool IsEmpty() const override;
  int GetDepth() const override;
  const char *GetTypeName() const override;
  std::string GetStats() const override;
  void DebugDraw(std::function<void(const AABB &, int depth)> drawCallback) override;

 private:
  /**
   * @brief 递归构建BVH树
   * @param nodes 场景节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param depth 当前深度
   * @return 构建的BVH节点
   */
  BVHNode *BuildTree(std::vector<SceneNode *> &nodes, int start, int end, int depth);

  /**
   * @brief 选择最佳分割轴和位置
   * @param nodes 节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param axis 最佳分割轴（输出参数）
   * @param splitPos 最佳分割位置（输出参数）
   * @return 是否找到有效分割
   */
  bool FindBestSplit(
      const std::vector<SceneNode *> &nodes, int start, int end, int &axis, float &splitPos) const;

  /**
   * @brief 按指定轴和位置分割节点列表
   * @param nodes 节点列表
   * @param start 起始索引
   * @param end 结束索引
   * @param axis 分割轴
   * @param splitPos 分割位置
   * @return 分割点索引
   */
  int PartitionNodes(
      std::vector<SceneNode *> &nodes, int start, int end, int axis, float splitPos) const;

  /**
   * @brief 递归释放BVH节点
   * @param node 要释放的节点
   */
  void FreeNode(BVHNode *node);

  /**
   * @brief 递归射线检测
   * @param node 当前节点
   * @param ray 射线
   * @param results 结果列表
   */
  void RaycastRecursive(BVHNode *node, const Ray &ray, std::vector<SceneNode *> &results) const;

  /**
   * @brief 递归寻找第一个射线相交
   * @param node 当前节点
   * @param ray 射线
   * @param bestNode 最佳节点（输出参数）
   * @param bestDistance 最佳距离（输出参数）
   */
  void RaycastFirstRecursive(BVHNode *node,
                             const Ray &ray,
                             SceneNode *&bestNode,
                             float &bestDistance) const;

  /**
   * @brief 递归视锥体裁剪
   * @param node 当前节点
   * @param frustum 视锥体
   * @param results 结果列表
   */
  void FrustumCullRecursive(BVHNode *node,
                            const Frustum &frustum,
                            std::vector<SceneNode *> &results) const;

  /**
   * @brief 递归球体查询
   * @param node 当前节点
   * @param sphere 球体
   * @param results 结果列表
   */
  void SphereQueryRecursive(BVHNode *node,
                            const Sphere &sphere,
                            std::vector<SceneNode *> &results) const;

  /**
   * @brief 递归AABB查询
   * @param node 当前节点
   * @param aabb AABB
   * @param results 结果列表
   */
  void AABBQueryRecursive(BVHNode *node,
                          const AABB &aabb,
                          std::vector<SceneNode *> &results) const;

  /**
   * @brief 递归遍历所有节点
   * @param node 当前节点
   * @param callback 回调函数
   * @return 是否继续遍历
   */
  bool ForEachNodeRecursive(BVHNode *node, std::function<bool(SceneNode *)> callback) const;

  /**
   * @brief 递归调试绘制
   * @param node 当前节点
   * @param drawCallback 绘制回调
   */
  void DebugDrawRecursive(BVHNode *node,
                          std::function<void(const AABB &, int depth)> drawCallback) const;

  /**
   * @brief 递归统计节点信息
   * @param node 当前节点
   * @param stats 统计信息
   */
  void CollectStatsRecursive(BVHNode *node, struct BVHStats &stats) const;

  /**
   * @brief 更新场景节点在BVH中的位置
   * @param node 场景节点
   */
  void UpdateNode(SceneNode *node);

 private:
  BVHNode *root_ = nullptr;            ///< BVH根节点
  std::vector<SceneNode *> allNodes_;  ///< 所有场景节点列表（用于快速重建）
  int maxDepth_;                       ///< 最大构建深度
  int minLeafSize_;                    ///< 叶子节点最小对象数
  bool needsRebuild_ = false;          ///< 需要重建标记
  size_t nodeCount_ = 0;               ///< 总节点数统计

  // 性能统计
  mutable uint64_t raycastTests_ = 0;  ///< 射线检测测试次数
  mutable uint64_t frustumTests_ = 0;  ///< 视锥体测试次数
};
}  // namespace mite

#endif  // MITE_SIMPLE_BVH_H
