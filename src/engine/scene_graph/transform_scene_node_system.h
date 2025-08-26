#ifndef MITE_TRANSFORM_SCENE_NODE_SYSTEM_H
#define MITE_TRANSFORM_SCENE_NODE_SYSTEM_H

#include "scene_core_components/hierarchy_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_node.h"

namespace mite {

/**
 * @class TransformSystem
 * @brief 变换系统 - 负责ECS组件与场景图节点之间的变换数据同步
 * 
 * 注意：
 * 与SceneCore模块的TransformComponentSystem不同，
 * 不负责TransformComponent的管理，仅负责Entity和
 * SceneNode之间的Transform数据传递与同步。
 *
 * 设计原则：
 * 1. 不维护SceneRegistry引用，完全通过事件和参数传递
 * 2. 使用延迟处理机制，在Update阶段批量执行
 * 3. 提供清晰的接口用于场景图集成
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

  // ==================== 场景图集成接口 ====================

  /**
   * @brief 注册场景节点映射
   * @param entity 实体
   * @param node 对应的场景节点
   */
  void RegisterSceneNode(Entity entity, SceneNode *node);

  /**
   * @brief 取消注册场景节点映射
   * @param entity 实体
   */
  void UnregisterSceneNode(Entity entity);

  /**
   * @brief 获取实体对应的场景节点
   * @param entity 实体句柄
   * @return 场景节点指针，不存在时返回nullptr
   */
  SceneNode *GetSceneNode(Entity entity) const;

  /**
   * @brief 批量同步ECS组件数据到场景节点
   * @param registry 场景注册表引用
   */
  void SyncAllComponentsToNodes(SceneRegistry &registry);

  /**
   * @brief 手动标记实体需要同步
   * @param entity 需要同步的实体
   */
  void MarkEntityForSync(Entity entity);

 private:
  // ==================== 事件处理回调 ====================
  bool OnTransformComponentAdded(ComponentAddedEvent<TransformComponent> &e);
  bool OnTransformComponentRemoved(ComponentRemovedEvent<TransformComponent> &e);
  bool OnTransformUpdated(TransformUpdatedEvent &e);
  bool OnParentChanged(ParentChangedEvent &e);

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
   * @param node 对应的场景节点
   */
  void SyncComponentToNode(SceneRegistry &registry, Entity entity, SceneNode *node);

 private:
  // 实体到场景节点的映射表
  std::unordered_map<Entity, SceneNode *> m_entityToNodeMap;

  // 需要同步的实体队列
  std::vector<Entity> m_pendingSyncEntities;

  // 线程安全保护
  mutable std::mutex m_mutex;
};

}  // namespace mite

#endif  // MITE_TRANSFORM_SYSTEM_H
