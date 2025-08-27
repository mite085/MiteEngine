#ifndef MITE_HIERARCHY_SCENE_NODE_SYSTEM_H
#define MITE_HIERARCHY_SCENE_NODE_SYSTEM_H

#include "scene_core/component_system.h"
#include "scene_core_components/hierarchy_component.h"
#include "scene_graph.h"

namespace mite {

/**
 * @class HierarchySceneNodeSystem
 * @brief 层级场景节点系统 - 专门处理ECS层级组件与场景图节点父子关系的同步
 *
 * 职责：
 * 1. 监听HierarchyComponent变化，维护SceneNode的父子关系
 * 2. 处理层级结构变化对场景图的影响
 * 3. 确保场景图节点结构与ECS层级结构一致
 */
class HierarchySceneNodeSystem : public ComponentSystem {
 public:
  DECLARE_COMPONENT_SYSTEM(HierarchySceneNodeSystem)

  HierarchySceneNodeSystem();
  ~HierarchySceneNodeSystem() override = default;

  // ==================== ComponentSystem 接口实现 ====================
  Component::Family GetExecutionOrder() const override;
  void Initialize() override;
  void Update(float deltaTime, SceneRegistry &registry) override;
  void Shutdown() override;

  std::vector<std::type_index> GetComponentTypes() const override;
  std::vector<std::type_index> GetSystemDependencies() const override;

  // ==================== SceneGraph 访问接口 ====================
  void SetSceneGraph(SceneGraph *sceneGraph);

 private:
  // ==================== 事件处理回调 ====================
  bool OnHierarchyComponentAdded(ComponentAddedEvent<HierarchyComponent> &e);
  bool OnHierarchyComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e);
  bool OnParentChanged(ParentChangedEvent &e);

  // ==================== 内部处理方法 ====================
  void UpdateSceneNodeParent(SceneRegistry &registry, Entity entity);
  void ProcessPendingHierarchyChanges(SceneRegistry &registry);

 private:
  SceneGraph *m_sceneGraph = nullptr;
  std::vector<Entity> m_pendingHierarchyChanges;
  SubscriptionGroup m_eventSubscriptions;
  Logger m_logger;
};

}  // namespace mite

#endif  // MITE_HIERARCHY_SCENE_NODE_SYSTEM_H
