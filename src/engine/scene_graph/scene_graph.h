#ifndef MITE_SCENE_GRAPH_H
#define MITE_SCENE_GRAPH_H

#include "scene_node_manager.h"
#include "spatial_partition.h"

namespace mite {
/**
 * @class SceneGraph（负责协调与暴露接口，不直接实现）
 * @brief 场景图独立服务 - 负责场景节点层级管理和空间查询优化
 *
 * 核心职责：
 * 1. 管理场景节点的层级树结构
 * 2. 维护空间划分数据结构（BVH、Octree等）
 * 3. 提供高效的空间查询接口
 * 4. 作为编辑器场景树的唯一数据源
 * 5. 协助SceneView进行视锥体裁剪优化
 */
class SceneGraph {
 public:
  /**
   * @brief 构造函数
   * @param spatialPartitionType 空间划分类型（默认BVH）
   */
  explicit SceneGraph();
  ~SceneGraph() = default;

  // 禁止拷贝和移动
  SceneGraph(const SceneGraph &) = delete;
  SceneGraph &operator=(const SceneGraph &) = delete;
  SceneGraph(SceneGraph &&) = delete;
  SceneGraph &operator=(SceneGraph &&) = delete;

  // 初始化与清理
  void Initialize();
  void CleanUp();

  // ==================== 场景节点生命周期管理 ====================
  /**
   * @brief 为实体创建场景节点
   * @param entity 目标实体
   * @return 创建的场景节点指针，失败返回nullptr
   */
  SceneNode *CreateNode(SceneRegistry &registry, Entity entity);
  /**
   * @brief 销毁实体的场景节点
   * @param entity 目标实体
   * @return 是否成功销毁
   */
  bool DestroyNode(SceneRegistry &registry, Entity entity);
  /**
   * @brief 重建空间划分结构
   */
  void RebuildSpatialPartition();

  // ==================== 场景节点查询接口 ====================
  /**
   * @brief 获取实体对应的场景节点
   * @param entity 实体句柄
   * @return 场景节点指针，不存在时返回nullptr
   */
  SceneNode *GetNode(Entity entity) const;
  /**
   * @brief 检查实体是否有对应的场景节点
   * @param entity 实体句柄
   * @return 是否存在场景节点
   */
  bool HasNode(Entity entity) const;
  /**
   * @brief 通过路径查找场景节点
   * @param path 节点路径
   * @return 场景节点指针
   */
  SceneNode *FindNodeByPath(const std::string &path) const;
  /**
   * @brief 获取根节点列表（没有父节点的节点）
   * @return 根节点指针列表
   */
  std::vector<SceneNode *> GetRootNodes() const;
  /**
   * @brief 获取所有场景节点
   * @return 所有场景节点指针列表
   */
  std::vector<SceneNode *> GetAllNodes() const;

  // ==================== 空间查询接口 ====================
  /**
   * @brief 视锥体裁剪查询
   * @param frustum 视锥体
   * @param visibleMask 可见性掩码（用于分层渲染）
   * @return 可见节点列表
   */
  std::vector<SceneNode *> FrustumCull(const Frustum &frustum, const uint32_t visibleMask) const;
  /**
   * @brief 射线检测查询
   * @param ray 检测射线
   * @return 相交节点列表
   */
  std::vector<SceneNode *> Raycast(const Ray &ray) const;
  /**
   * @brief 体积查询
   * @param volume 查询体积
   * @return 包含节点列表
   */
  std::vector<SceneNode *> VolumeQuery(const BoundingVolume &volume) const;
  /**
   * @brief 点查询
   * @param point 查询点
   * @return 包含节点列表
   */
  std::vector<SceneNode *> PointQuery(const glm::vec3 &point) const;

  // ==================== 场景图遍历接口 ====================
  /**
   * @brief 遍历场景图
   * @param callback 回调函数
   * @param type 遍历类型
   */
  void Traverse(std::function<bool(SceneNode *)> callback,
                SceneNodeManager::TraversalType type =
                    SceneNodeManager::TraversalType::DepthFirstPreOrder) const;
  /**
   * @brief 遍历可见节点
   * @param callback 回调函数
   * @param type 遍历类型
   */
  void TraverseVisible(std::function<bool(SceneNode *)> callback,
                       SceneNodeManager::TraversalType type =
                           SceneNodeManager::TraversalType::DepthFirstPreOrder) const;

  // ==================== 更新管理接口 ====================

  /**
   * @brief 标记节点需要更新
   * @param entity ECS实体
   * @param recursive 是否递归标记子节点
   */
  void MarkDirty(Entity entity, bool recursive = false);

  /**
   * @brief 更新场景图
   * @param registry ECS注册表
   */
  void Update(SceneRegistry &registry);

  // ==================== 状态查询接口 ====================

  /**
   * @brief 获取场景图节点数量
   * @return 节点总数
   */
  size_t GetNodeCount() const;

  /**
   * @brief 判断场景图是否为空
   * @return 是否为空
   */
  bool IsEmpty() const;

  /**
   * @brief 获取统计信息
   * @return 统计信息字符串
   */
  std::string GetStats() const;

  // ==================== 调试接口 ====================
  /**
   * @brief 调试绘制
   * @param drawCallback 绘制回调函数
   */
  void DebugDraw(std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback) const;

 private:
  // ==================== 事件响应 ====================
  /**
   * @brief 处理实体创建事件
   * @param event 实体创建事件
   */
  void OnEntityCreated(EntityCreatedEvent &event);
  /**
   * @brief 处理实体销毁事件
   * @param event 实体销毁事件
   */
  void OnEntityDestroyed(EntityDestroyedEvent &event);
  /**
   * @brief 处理组件添加事件（用于检测必要组件）
   * @param event 组件添加事件
   */
  void OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &event);
  void OnBoundingVolumeComponentAdded(ComponentAddedEvent<BoundingVolumeComponent> &event);
  /**
   * @brief 执行延迟的节点创建与销毁
   */
  void ProcessScheduledCreationsAndDestruction(SceneRegistry &registry);

 private:
  std::unique_ptr<SceneNodeManager> m_NodeManager;
  std::unique_ptr<SpatialPartition> m_SpatialPartition;

  
  struct EntityComponents {
    bool hasTransform = false;
    bool hasBoundingVolume = false;
  };
  std::unordered_map<Entity, EntityComponents> m_PendingCreateNodes;  // 待创建的实体队列
  std::unordered_set<Entity> m_PendingDestroyNodes;                   // 待销毁的实体队列

  // 事件订阅
  SubscriptionGroup m_EventSubscriptions;
};
}  // namespace mite

#endif  // MITE_SCENE_GRAPH_H
