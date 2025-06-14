#ifndef MITE_SCENE_GRAPH
#define MITE_SCENE_GRAPH

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
/**
 * @class SceneGraph
 * @brief 管理场景中实体的层次结构关系，提供高效的遍历和查询功能
 *
 * 基于EnTT ECS和HierarchyComponent构建，支持：
 * - 层次结构的维护和验证
 * - 多种遍历方式(DFS/BFS)
 * - 空间变换继承
 * - 可见性继承
 * - 场景图事件通知
 */
class SceneGraph {
 public:
  // 遍历顺序枚举
  enum class TraversalOrder {
    DepthFirst,         // 深度优先
    BreadthFirst,       // 广度优先
    ReverseDepthFirst,  // 逆深度优先(从叶子到根)
  };

  // 遍历回调函数类型
  using VisitorFunc = std::function<bool(entt::entity)>;

  /**
   * @brief 构造函数
   * @param registry EnTT注册表引用
   */
  explicit SceneGraph(entt::registry &registry) : m_Registry(registry) {}

  // 禁止拷贝
  SceneGraph(const SceneGraph &) = delete;
  SceneGraph &operator=(const SceneGraph &) = delete;

  /**
   * @brief 设置实体父节点
   * @param entity 目标实体
   * @param newParent 新父实体(entt::null表示设为根节点)
   * @return 是否设置成功
   *
   * @note 会进行循环依赖检查，如果形成循环则操作失败
   */
  bool SetParent(entt::entity entity, entt::entity newParent);

  /**
   * @brief 获取实体父节点
   * @param entity 目标实体
   * @return 父实体句柄(entt::null表示无父节点)
   */
  entt::entity GetParent(entt::entity entity) const;

  /**
   * @brief 获取实体子节点列表
   * @param entity 目标实体
   * @return 子实体列表(按添加顺序)
   */
  const std::vector<entt::entity> &GetChildren(entt::entity entity) const;

  /**
   * @brief 检查实体是否为根节点
   * @param entity 目标实体
   */
  bool IsRoot(entt::entity entity) const;

  /**
   * @brief 检查实体是否为叶节点
   * @param entity 目标实体
   */
  bool IsLeaf(entt::entity entity) const;

  /**
   * @brief 获取实体在场景图中的深度
   * @param entity 目标实体
   * @return 深度值(根节点为0)
   */
  size_t GetDepth(entt::entity entity) const;

  /**
   * @brief 遍历场景图
   * @param root 起始实体
   * @param visitor 访问者回调函数
   * @param order 遍历顺序
   *
   * @note 如果visitor返回false，则停止遍历
   */
  void Traverse(entt::entity root,
                const VisitorFunc &visitor,
                TraversalOrder order = TraversalOrder::DepthFirst) const;

  /**
   * @brief 遍历整个场景图(从所有根节点开始)
   * @param visitor 访问者回调函数
   * @param order 遍历顺序
   */
  void TraverseAll(const VisitorFunc &visitor,
                   TraversalOrder order = TraversalOrder::DepthFirst) const;

  /**
   * @brief 获取从实体到根节点的路径
   * @param entity 起始实体
   * @return 路径实体列表(从实体到根节点顺序)
   */
  std::vector<entt::entity> GetPathToRoot(entt::entity entity) const;

  /**
   * @brief 检查两个实体是否在同一个层次结构中
   * @param entity1 实体1
   * @param entity2 实体2
   */
  bool IsInSameHierarchy(entt::entity entity1, entt::entity entity2) const;

  /**
   * @brief 获取场景中所有根实体
   * @return 根实体列表
   */
  std::vector<entt::entity> GetRoots() const;

  /**
   * @brief 重新计算并缓存所有实体的深度值
   *
   * @note 通常在批量修改层次结构后调用以提高后续查询性能
   */
  void RecalculateAllDepths();

  /**
   * @brief 每帧更新场景图状态
   * @param timestep 帧时间间隔(秒)
   *
   * 主要功能:
   * 1. 传播变换更新(从脏标记的父节点到子节点)
   * 2. 更新可见性状态
   * 3. 维护其他需要每帧更新的场景图状态
   * 4. 处理延迟的层次结构变更
   */
  void SceneGraph::OnUpdate(float timestep);

 private:
  /**
   * @brief 内部方法 - 检查是否形成循环依赖
   * @param child 子实体
   * @param newParent 新父实体
   */
  bool WouldFormCycle(entt::entity child, entt::entity newParent) const;

  /**
   * @brief 内部方法 - 深度优先遍历实现
   * @param entity 当前实体
   * @param visitor 访问者回调
   * @return 是否继续遍历
   */
  bool TraverseDFS(entt::entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief 内部方法 - 广度优先遍历实现
   * @param entity 起始实体
   * @param visitor 访问者回调
   */
  void TraverseBFS(entt::entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief 内部方法 - 逆深度优先遍历实现
   * @param entity 当前实体
   * @param visitor 访问者回调
   * @return 是否继续遍历
   */
  bool TraverseReverseDFS(entt::entity entity, const VisitorFunc &visitor) const;

  // EnTT注册表引用
  entt::registry &m_Registry;
};
};  // namespace mite

#endif
