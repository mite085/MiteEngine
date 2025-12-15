#ifndef MITE_SCENE_NODE_MANAGER_H
#define MITE_SCENE_NODE_MANAGER_H

#include "scene_node.h"
#include "spatial_partition.h"

#include "scene_core_components/bounding_volume_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_core_components/visibility_component.h"
#include "scene_core_components/light_component.h"

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
   * @param entity 目标实体的Parent，若为根节点则输入空实体，默认Parent为空实体
   * @return 创建的场景节点指针，失败返回nullptr
   */
  std::shared_ptr<SceneNode> CreateNode(SceneRegistry &registry, Entity entity, Entity parent = Entity{});
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
  std::shared_ptr<SceneNode> GetNode(Entity entity) const;
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
  std::vector<std::shared_ptr<SceneNode> > GetRootNodes() const;
  /**
   * @brief 获取所有场景节点
   * @return 所有场景节点指针列表
   */
  std::vector<std::shared_ptr<SceneNode> > GetAllNodes() const;
  /**
   * @brief 获取所有光源节点
   * @return 所有光源节点指针列表
   */
  std::vector<std::shared_ptr<SceneNode> > GetLightNodes() const;
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
  std::string GetNodePath(std::shared_ptr<SceneNode> node) const;
  /**
   * @brief 通过路径查找场景节点（与GetNodePath实现自洽）
   * @param path 节点路径
   * @return 场景节点指针，找不到返回nullptr
   */
  std::shared_ptr<SceneNode> FindNodeByPath(const std::string &path) const;
  /**
   * @brief 遍历场景树执行回调函数
   * @param callback 回调函数，返回false可中断遍历
   * @param traversalType 遍历类型，默认为深度优先前序遍历
   */
  void TraverseTree(std::function<bool(std::shared_ptr<SceneNode> )> callback,
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
  bool SetParent(std::shared_ptr<SceneNode> node, std::shared_ptr<SceneNode> newParent);
  /**
   * @brief 标记节点需要更新（变换或包围盒变化）
   * @param entity 目标实体
   * 
   * 分为仅标记当前节点，和递归标记所有子节点，两种模式
   */
  void MarkNodeDirty(std::shared_ptr<SceneNode> node);
  void MarkNodeDirtyRecursive(std::shared_ptr<SceneNode> node);
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
      std::shared_ptr<SceneNode> node,
      std::function<bool(std::shared_ptr<SceneNode> )> callback) const;  // 深度优先前序遍历（根-左-右）
  bool TraverseDepthFirstPostOrder(
      std::shared_ptr<SceneNode> node,
      std::function<bool(std::shared_ptr<SceneNode> )> callback) const;  // 深度优先后序遍历（左-右-根）
  bool TraverseBreadthFirst(std::shared_ptr<SceneNode> node, std::function<bool(std::shared_ptr<SceneNode> )> callback)
      const;  // 广度优先遍历（层级遍历，从根到底层）
  bool TraverseReverseBreadthFirst(std::shared_ptr<SceneNode> node,
                                   std::function<bool(std::shared_ptr<SceneNode> )> callback)
      const;  // 反向广度优先遍历（层级遍历，从底层到根）

  /**
   * @brief 验证父子关系是否有效（防止循环引用）
   */
  bool ValidateParenting(std::shared_ptr<SceneNode> node, std::shared_ptr<SceneNode> newParent) const;
  /**
   * @brief 构建路径缓存（惰性更新）
   */
  void BuildPathCache() const;
  /**
   * @brief 计算节点的完整路径
   */
  std::string CalculateNodePath(std::shared_ptr<SceneNode> node) const;

  // ==================== 事件消费 ====================
  void OnTransformComponentUpdated(TransformUpdatedEvent& e);
  void OnBoundingVolumeComponentUpdated(BoundingVolumeChangedEvent &e);
  void OnVisibilityComponentUpdated(VisibilityChangedEvent &e);
  void OnSceneNodeParentChange(SceneNodeParentChangeEvent &e);

 private:
  // 实体到场景节点的映射表
  std::unordered_map<Entity, std::shared_ptr<SceneNode>> m_EntityToNodeMap;

  // 光源节点列表（只要包含Light组件即可认为是光照节点）
  std::unordered_set<std::shared_ptr<SceneNode> > m_LightNodes;

  // 需要更新的脏节点列表
  std::unordered_set<std::shared_ptr<SceneNode> > m_DirtyNodes;

  // 空间划分结构
  SpatialPartition& m_SpatialPartition;

  // 路径到节点的映射缓存
  mutable std::unordered_map<std::string, std::shared_ptr<SceneNode> > m_PathToNodeCache;
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