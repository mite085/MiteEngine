#ifndef MITE_SCENE_GRAPH_SYSTEM_H
#define MITE_SCENE_GRAPH_SYSTEM_H

#include "scene_core_components/mesh_component.h"
#include "transform_scene_node_system.h"
#include "scene_graph.h"

namespace mite {
/**
 * @class SceneGraphSystem
 * @brief 场景图系统 - 负责ECS与SceneGraph之间的数据同步和协调
 *
 * 核心职责：
 * 1. 监听ECS事件，维护SceneGraph中的节点生命周期
 * 2. 协调TransformSystem进行数据同步
 * 3. 处理组件变化对场景图的影响
 * 4. 作为ECS与SceneGraph之间的桥梁
 *
 * 设计原则：
 * - 纯粹的ComponentSystem，不包含场景管理逻辑
 * - 专注于ECS事件响应和数据同步
 * - 依赖SceneGraph服务完成实际场景操作
 */
class SceneGraphSystem : public ComponentSystem {
 public:
  DECLARE_COMPONENT_SYSTEM(SceneGraphSystem)

  SceneGraphSystem();
  ~SceneGraphSystem() override = default;

  // ==================== ComponentSystem 接口实现 ====================
  Component::Family GetExecutionOrder() const override;
  void Initialize() override;
  void Update(float deltaTime, SceneRegistry &registry) override;
  void Shutdown() override;

  std::vector<std::type_index> GetComponentTypes() const override;
  std::vector<std::type_index> GetSystemDependencies() const override;

  // ==================== SceneGraph 访问接口 ====================

  /**
   * @brief 获取SceneGraph服务实例
   * @return SceneGraph指针，可能为nullptr（如果未初始化）
   */
  SceneGraph *GetSceneGraph() const;

  /**
   * @brief 设置外部SceneGraph服务实例
   * @param sceneGraph 外部的SceneGraph实例
   */
  void SetSceneGraph(SceneGraph *sceneGraph);

  // ==================== 调试和统计接口 ====================

  /**
   * @brief 获取系统统计信息
   * @return 统计信息字符串
   */
  std::string GetStats() const;

 private:
  // ==================== ECS事件处理回调 ====================

  /**
   * @brief 处理实体创建事件
   */
  bool OnEntityCreated(EntityCreatedEvent &e);

  /**
   * @brief 处理实体销毁事件
   */
  bool OnEntityDestroyed(EntityDestroyedEvent &e);

  /**
   * @brief 处理变换组件添加事件
   */
  bool OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e);

  /**
   * @brief 处理变换组件移除事件
   */
  bool OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e);

  /**
   * @brief 处理变换更新事件
   */
  bool OnTransformUpdated(TransformUpdatedEvent &e);

  /**
   * @brief 处理Mesh组件添加事件
   */
  bool OnMeshComponentAdded(ComponentAddedEvent<MeshComponent> &e);

  /**
   * @brief 处理Mesh组件移除事件
   */
  bool OnMeshComponentRemoved(ComponentRemovedEvent<MeshComponent> &e);

  /**
   * @brief 处理父节点改变事件
   */
  bool OnParentChanged(ParentChangedEvent &e);

  // ==================== 内部工具方法 ====================

  /**
   * @brief 为实体创建场景节点（如果满足条件）
   * @param entity 目标实体
   */
  void CreateNodeForEntity(SceneRegistry &registry, Entity entity);

  /**
   * @brief 检查实体是否需要场景节点
   * @param entity 目标实体
   * @return 是否需要创建场景节点
   */
  bool ShouldCreateNodeForEntity(SceneRegistry &registry, Entity entity) const;

  /**
   * @brief 同步变换数据到SceneGraph
   * @param entity 目标实体
   */
  void SyncTransformToSceneGraph(SceneRegistry &registry, Entity entity);

  /**
   * @brief 同步包围盒数据到SceneGraph
   * @param entity 目标实体
   */
  void SyncBoundsToSceneGraph(SceneRegistry &registry, Entity entity);

  /**
   * @brief 处理层级关系变化
   * @param entity 目标实体
   * @param newParent 新的父实体
   */
  void HandleParentChange(SceneRegistry &registry, Entity entity, Entity newParent);

 private:
  /**
   * @brief 处理实体暂存队列
   * @param registry 注册表
   */
  void ProcessPendingOperations(SceneRegistry &registry);

  // 实体暂存队列
  std::vector<Entity> m_pendingCreateNodes;                       // 待创建的节点
  std::vector<Entity> m_pendingDestroyNodes;                      // 待销毁的节点
  std::vector<Entity> m_pendingSyncTransforms;                    // 待同步变换的节点
  std::vector<Entity> m_pendingSyncBounds;                        // 待同步包围盒的节点
  std::vector<std::pair<Entity, Entity>> m_pendingParentChanges;  // 待处理的父子关系变化

 private:
  // SceneGraph服务引用（外部注入）
  SceneGraph *m_sceneGraph;

  // 事件订阅管理
  SubscriptionGroup m_eventSubscriptions;

  // 性能统计
  struct {
    uint32_t nodesCreated = 0;
    uint32_t nodesDestroyed = 0;
    uint32_t transformSyncs = 0;
    uint32_t boundsSyncs = 0;
  } m_stats;

  // 日志器
  Logger m_logger;
};
}  // namespace mite

#endif  // MITE_SCENE_GRAPH_SYSTEM_H
