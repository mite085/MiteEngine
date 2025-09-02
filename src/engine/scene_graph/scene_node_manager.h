#ifndef MITE_SCENE_NODE_MANAGER_H
#define MITE_SCENE_NODE_MANAGER_H

#include "scene_node.h"
#include "spatial_partition_manager.h"

namespace mite {
// 前向声明
class SceneRegistry;

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
  SceneNodeManager(SpatialPartitionManager &spatialPartition);
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
   * @brief 通过路径查找场景节点
   * @param path 节点路径
   * @return 场景节点指针，找不到返回nullptr
   */
  SceneNode *FindNodeByPath(const std::string &path) const;

  /**
   * @brief 遍历场景树执行回调函数
   * @param callback 回调函数，返回false可中断遍历
   */
  void TraverseTree(std::function<bool(SceneNode *)> callback) const;

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
   * @brief 更新场景节点的包围盒数据
   * @param entity 目标实体
   * @param localBounds 局部包围盒
   */
  void UpdateNodeBounds(SceneRegistry &registry, Entity entity, const AABB &localBounds);

  /**
   * @brief 标记节点需要更新（变换或包围盒变化）
   * @param entity 目标实体
   */
  void MarkNodeDirty(Entity entity);

  /**
   * @brief 批量更新所有脏节点
   */
  void Update(SceneRegistry &registry);

 private:
  // ==================== 内部工具方法 ====================
  /**
   * @brief 递归遍历场景树辅助函数
   */
  bool TraverseRecursive(SceneNode *node, std::function<bool(SceneNode *)> callback) const;

  /**
   * @brief 验证父子关系是否有效（防止循环引用）
   */
  bool ValidateParenting(SceneNode *node, SceneNode *newParent) const;

  // 实体到场景节点的映射表
  std::unordered_map<Entity, std::unique_ptr<SceneNode>> m_entityToNodeMap;

  // 需要更新的脏节点列表
  std::vector<Entity> m_dirtyNodes;

  // 空间划分结构
  SpatialPartitionManager &m_spatialPartition;

  // 线程安全保护
  mutable std::mutex m_mutex;

  // 日志器
  Logger m_Logger;
};
}  // namespace mite

#endif  // MITE_SCENE_NODE_MANAGER_H