#ifndef MITE_SCENE_NODE_MANAGER_H
#define MITE_SCENE_NODE_MANAGER_H

#include "scene_node.h"
#include "spatial_partition.h"

#include "scene_core_components/bounding_volume_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_core_components/visibility_component.h"

namespace mite {
/**
 * @class SceneNodeManager
 * @brief 负责场景节点的生命周期管理
 *
 * 场景节点更新必然会引起空间划分结构更新，
 * 所以应当依赖SpatialPartitionManager，
 * 使用依赖注入的方式进行构造
 */
class SceneNodeManager {
 public:
  /**
   * @enum TraversalType
   * @brief 场景树遍历类型枚举
   */
  enum class TraversalType {
    DepthFirstPreOrder,   // 深度优先前序遍历（根-左-右）
    DepthFirstPostOrder,  // 深度优先后序遍历（左-右-根）
    BreadthFirst,         // 广度优先遍历（层级遍历）
    ReverseBreadthFirst   // 反向广度优先遍历（从底层到根）
  };

  SceneNodeManager(SpatialPartition &spatialPartition);
  ~SceneNodeManager() = default;
  void Clear();

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
   * @brief 获取根节点列表（没有父节点的节点）
   * @return 根节点指针列表
   */
  std::vector<SceneNode *> GetRootNodes() const;
  /**
   * @brief 获取所有场景节点
   * @return 所有场景节点指针列表
   */
  std::vector<SceneNode *> GetAllNodes() const;
  /**
   * @brief 获取场景节点数量
   * @return 节点总数
   */
  size_t GetNodeCount() const;
  /**
   * @brief 获取节点的完整路径（用于编辑器序列化）
   * @param node 目标节点
   * @return 节点路径字符串（如"Root/Camera/Light"）
   */
  std::string GetNodePath(SceneNode *node) const;
  /**
   * @brief 通过路径查找场景节点（与GetNodePath实现自洽）
   * @param path 节点路径
   * @return 场景节点指针，找不到返回nullptr
   */
  SceneNode *FindNodeByPath(const std::string &path) const;
  /**
   * @brief 遍历场景树执行回调函数
   * @param callback 回调函数，返回false可中断遍历
   * @param traversalType 遍历类型，默认为深度优先前序遍历
   */
  void TraverseTree(std::function<bool(SceneNode *)> callback,
                    TraversalType traversalType = TraversalType::DepthFirstPreOrder) const;

  /**
   * @brief 判断场景图是否为空
   * @return 是否为空
   */
  bool IsEmpty() const;

  // ==================== 节点更新接口 ====================
  /**
   * @brief 设置节点的父节点
   * @param node 目标节点
   * @param newParent 新的父节点（nullptr表示设为根节点）
   * @return 是否成功设置
   */
  bool SetParent(SceneNode *node, SceneNode *newParent);
  /**
   * @brief 标记节点需要更新（变换或包围盒变化）
   * @param entity 目标实体
   * 
   * 分为仅标记当前节点，和递归标记所有子节点，两种模式
   */
  void MarkNodeDirty(Entity entity);
  void MarkNodeDirtyRecursive(Entity entity);
  /**
   * @brief 批量更新所有脏节点（每帧执行）
   */
  void Update(SceneRegistry &registry);

 private:
  // ==================== 内部工具方法 ====================
  /**
   * @brief 递归遍历场景树辅助函数
   */
  bool TraverseDepthFirstPreOrder(
      SceneNode *node,
      std::function<bool(SceneNode *)> callback) const;  // 深度优先前序遍历（根-左-右）
  bool TraverseDepthFirstPostOrder(
      SceneNode *node,
      std::function<bool(SceneNode *)> callback) const;  // 深度优先后序遍历（左-右-根）
  bool TraverseBreadthFirst(SceneNode *node, std::function<bool(SceneNode *)> callback)
      const;  // 广度优先遍历（层级遍历，从根到底层）
  bool TraverseReverseBreadthFirst(SceneNode *node,
                                   std::function<bool(SceneNode *)> callback)
      const;  // 反向广度优先遍历（层级遍历，从底层到根）

  /**
   * @brief 验证父子关系是否有效（防止循环引用）
   */
  bool ValidateParenting(SceneNode *node, SceneNode *newParent) const;
  /**
   * @brief 构建路径缓存（惰性更新）
   */
  void BuildPathCache() const;
  /**
   * @brief 计算节点的完整路径
   */
  std::string CalculateNodePath(SceneNode *node) const;

  // ==================== 事件消费 ====================
  /**
   * @brief 处理Transform组件更新事件
   */
  void OnTransformComponentUpdated(TransformUpdatedEvent& e);
  /**
   * @brief 处理BoundingVolume组件更新事件
   */
  void OnBoundingVolumeComponentUpdated(BoundingVolumeChangedEvent &e);
  /**
   * @brief 处理Visibility组件更新事件
   */
  void OnVisibilityComponentUpdated(VisibilityChangedEvent &e);

 private:
  // 实体到场景节点的映射表
  std::unordered_map<Entity, std::unique_ptr<SceneNode>> m_EntityToNodeMap;

  // 需要更新的脏节点列表
  std::unordered_set<Entity> m_DirtyNodes;

  // 空间划分结构
  SpatialPartition& m_SpatialPartition;

  // 路径到节点的映射缓存
  mutable std::unordered_map<std::string, SceneNode *> m_PathToNodeCache;
  mutable bool m_PathCacheDirty = true;  // 路径缓存脏标记

  // 线程安全保护
  mutable std::mutex m_Mutex;

  // 日志器
  Logger m_Logger;

  // 事件订阅
  SubscriptionGroup m_EventSubscriptions; 
};
}  // namespace mite

#endif  // MITE_SCENE_NODE_MANAGER_H