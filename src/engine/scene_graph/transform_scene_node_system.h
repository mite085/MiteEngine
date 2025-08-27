#ifndef MITE_TRANSFORM_SCENE_NODE_SYSTEM_H
#define MITE_TRANSFORM_SCENE_NODE_SYSTEM_H

#include "scene_core_components/hierarchy_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_graph.h"

namespace mite {
/**
 * @class TransformSceneNodeSystem
 * @brief 变换场景节点系统 - 负责ECS变换组件与场景图节点之间的数据同步
 *
 * 注意：
 * 与SceneCore模块的TransformComponentSystem不同，
 * 不负责TransformComponent的管理，仅负责Entity和
 * SceneNode之间的Transform数据传递与同步。
 * 
 * 设计原则：
 * 1. 通过SceneGraph服务直接查询SceneNode，避免维护重复映射
 * 2. 专注于变换数据同步，不涉及层级关系处理
 * 3. 使用延迟处理机制提高性能
 */
class TransformSceneNodeSystem : public ComponentSystem {
 public:
  DECLARE_COMPONENT_SYSTEM(TransformSceneNodeSystem)
  TransformSceneNodeSystem();
  ~TransformSceneNodeSystem() override = default;

  // ==================== ComponentSystem 接口实现 ====================
  Component::Family GetExecutionOrder() const override;
  void Initialize() override;
  void Update(float deltaTime, SceneRegistry &registry) override;
  void Shutdown() override;
  std::vector<std::type_index> GetComponentTypes() const override;
  std::vector<std::type_index> GetSystemDependencies() const override;

  // ==================== SceneGraph 访问接口 ====================
  void SetSceneGraph(SceneGraph *sceneGraph);

  // ==================== 同步控制接口 ====================
  /**
   * @brief 手动标记实体需要同步
   * @param entity 需要同步的实体
   */
  void MarkEntityForSync(Entity entity);
  /**
   * @brief 批量同步ECS组件数据到场景节点
   * @param registry 场景注册表引用
   */
  void SyncAllComponentsToNodes(SceneRegistry &registry);

 private:
  // ==================== 事件处理回调 ====================
  bool OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e);
  bool OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e);
  bool OnTransformUpdated(TransformUpdatedEvent &e);

  // ==================== 内部处理方法 ====================
  /**
   * @brief 处理待同步的实体
   * @param registry 场景注册表引用
   */
  void ProcessPendingSync(SceneRegistry &registry);
  /**
   * @brief 将ECS变换组件数据同步到场景节点
   * @param registry 场景注册表引用
   * @param entity 目标实体
   */
  void SyncComponentToNode(SceneRegistry &registry, Entity entity);

 private:
  SceneGraph *m_sceneGraph = nullptr;

  // 需要同步的实体队列
  std::vector<Entity> m_pendingSyncEntities;

  // PendingSyncEntities的线程安全保护
  mutable std::mutex m_mutex;
};

}  // namespace mite

#endif  // MITE_TRANSFORM_SYSTEM_H
