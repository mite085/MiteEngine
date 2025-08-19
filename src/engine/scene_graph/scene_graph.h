#ifndef MITE_SCENE_GRAPH
#define MITE_SCENE_GRAPH

#include "scene_core/scene_registry.h"
#include "scene_core_components/component_headers.h"

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
  using VisitorFunc = std::function<bool(Entity)>;

  SceneGraph();
  ~SceneGraph();

  // 禁止拷贝
  SceneGraph(const SceneGraph &) = delete;
  SceneGraph &operator=(const SceneGraph &) = delete;


  /**
   * @brief 初始化场景图系统
   * @note 注册所有必要的事件监听器
   * @param registry EnTT注册表引用
   */
  void Initialize(SceneRegistry &registry);

  /**
   * @brief 重置场景图状态
   */
  void Clear();

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

  /**
   * @brief 设置实体父节点
   * @param entity 目标实体
   * @param newParent 新父实体(Entity()表示设为根节点)
   * @return 是否设置成功
   *
   * @note 会进行循环依赖检查，如果形成循环则操作失败
   */
  bool SetParent(Entity entity, Entity newParent);

  /**
   * @brief 获取实体父节点
   * @param entity 目标实体
   * @return 父实体句柄(Entity()表示无父节点)
   */
  Entity GetParent(Entity entity) const;

  /**
   * @brief 获取实体子节点列表
   * @param entity 目标实体
   * @return 子实体列表(按添加顺序)
   */
  const std::vector<Entity> &GetChildren(Entity entity) const;

  /**
   * @brief 检查实体是否为根节点
   * @param entity 目标实体
   */
  bool IsRoot(Entity entity) const;

  /**
   * @brief 检查实体是否为叶节点
   * @param entity 目标实体
   */
  bool IsLeaf(Entity entity) const;

  /**
   * @brief 获取实体在场景图中的深度
   * @param entity 目标实体
   * @return 深度值(根节点为0)
   * 
   * 由于hierarchy->GetDepth伴随着
   * 深度信息更新，所以不能使用const限定符
   */
  size_t GetDepth(Entity entity);

  /**
   * @brief 遍历场景图
   * @param root 起始实体
   * @param visitor 访问者回调函数
   * @param order 遍历顺序
   *
   * @note 如果visitor返回false，则停止遍历
   */
  void Traverse(Entity root,
                const VisitorFunc &visitor,
                TraversalOrder order = TraversalOrder::DepthFirst) const;

  /**
   * @brief 遍历整个场景图(从所有根节点开始)
   * @param visitor 访问者回调函数
   * @param order 遍历顺序
   */
  void TraverseAll(const VisitorFunc &visitor,
                   TraversalOrder order = TraversalOrder::DepthFirst);

  /**
   * @brief 获取从实体到根节点的路径
   * @param entity 起始实体
   * @return 路径实体列表(从实体到根节点顺序)
   */
  std::vector<Entity> GetPathToRoot(Entity entity) const;

  /**
   * @brief 检查两个实体是否在同一个层次结构中
   * @param entity1 实体1
   * @param entity2 实体2
   */
  bool IsInSameHierarchy(Entity entity1, Entity entity2) const;

  /**
   * @brief 获取场景中所有根实体
   * @return 根实体列表
   * 
   * 由于GetRegistry().GetAllEntities()伴随着
   * storage信息查询，所以不能使用const限定符
   */
  std::vector<Entity> GetRoots();

  /**
   * @brief 重新计算并缓存所有实体的深度值
   *
   * @note 通常在批量修改层次结构后调用以提高后续查询性能
   */
  void RecalculateAllDepths();

  /**
   * @brief 渲染准备阶段回调，将场景图数据组织为渲染友好格式
   * 
   * TODO: 典型工作：
   * 1. 更新所有脏节点的世界变换
   * 2. 执行视锥体剔除
   * 3. 生成渲染队列
   * 4. 重置脏标记
   */
  void OnRenderPrepare();

 private:
  /**
   * @brief 获取Register的引用
   */
  SceneRegistry &GetRegistry()
  {
    return m_Registry.value();
  }
  const SceneRegistry &GetRegistry() const
  {
    return m_Registry.value();
  }

  /**
   * @brief 内部方法 - 深度优先遍历实现
   * @param entity 当前实体
   * @param visitor 访问者回调
   * @return 是否继续遍历
   */
  bool TraverseDFS(Entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief 内部方法 - 广度优先遍历实现
   * @param entity 起始实体
   * @param visitor 访问者回调
   */
  void TraverseBFS(Entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief 内部方法 - 逆深度优先遍历实现
   * @param entity 当前实体
   * @param visitor 访问者回调
   * @return 是否继续遍历
   */
  bool TraverseReverseDFS(Entity entity, const VisitorFunc &visitor) const;

  // 事件处理函数 ==========================================

  /**
   * @brief 处理实体创建事件
   */
  bool OnEntityCreated(EntityCreatedEvent &e);

  /**
   * @brief 处理实体销毁事件
   */
  bool OnEntityDestroyed(EntityDestroyedEvent &e);

  /**
   * @brief 处理层次组件添加事件
   */
  bool OnHierarchyAdded(ComponentAddedEvent<HierarchyComponent> &e);

  /**
   * @brief 处理层次组件变更事件
   */
  //bool OnHierarchyChanged(ComponentChangedEvent<HierarchyComponent> &e);

  /**
   * @brief 处理层次组件移除事件
   */
  bool OnHierarchyRemoved(ComponentRemovedEvent<HierarchyComponent> &e);

  /**
   * @brief 更新实体及其所有子代的深度缓存
   * @param entity 起始实体
   */
  void UpdateDepthCacheRecursive(Entity entity);

  /**
   * @brief 验证父子关系是否有效（防止循环引用）
   */
  bool ValidateHierarchy(Entity child, Entity newParent) const;

  /**
   * @brief 处理变换组件变更事件
   */
  bool OnTransformUpdated(TransformUpdatedEvent &e);

    /**
   * @brief 处理位置变更事件
   */
  bool OnPositionChanged(PositionChangedEvent &e);

  /**
   * @brief 处理旋转变更事件
   */
  bool OnRotationChanged(RotationChangedEvent &e);

  /**
   * @brief 处理缩放变更事件
   */
  bool OnScaleChanged(ScaleChangedEvent &e);

  /**
   * @brief 处理整体变换变更事件
   */
  bool OnTransformChanged(TransformChangedEvent &e);

  /**
   * @brief 标记子实体变换为脏
   * @param entity 父实体
   * @param flags 脏标记类型
   */
  void MarkChildrenDirty(Entity entity, uint8_t flags);



  // SceneRegistry注册表引用
  // 
  // 注意：
  // 此处使用optional包装的reference_wrapper，
  // 以实现延时引用的功能，目的是将ComponentSystem的
  // 构造和利用SceneRegistry&执行的初始化隔离开。
  std::optional<std::reference_wrapper<SceneRegistry>> m_Registry;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};
};  // namespace mite

#endif
