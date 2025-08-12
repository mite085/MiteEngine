#ifndef MITE_SCENE_VIEW
#define MITE_SCENE_VIEW

#include "renderable_item.h"
#include "scene_core/scene_event.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/component_headers.h"

namespace mite {
class SceneView {
 public:
  /**
   * 构造函数：需要传入SceneCore的ECS注册表和事件总线
   * @param registry EnTT注册表引用（由SceneCore持有）
   * @param eventBus 事件总线引用（用于订阅场景变更事件）
   */
  explicit SceneView(SceneRegistry &registry);
  ~SceneView();

  /**
   * 每帧更新渲染数据（在Application主循环中调用）
   * 1. 处理ECS事件（实体增删改）
   * 2. 更新内部渲染队列
   */
  void Update();

  /**
   * 获取当前帧的可渲染实体列表（供Renderer模块调用）
   * @return 只读的渲染队列引用，避免数据拷贝
   */
  const std::vector<std::shared_ptr<RenderableItem>> &GetRenderQueue() const;

 private:
  //=== 事件处理函数（订阅SceneCore的事件） ===//
  void OnEntityCreated(EntityCreatedEvent &event);
  void OnEntityDestroyed(EntityDestroyedEvent &event);
  void OnTransformChanged(TransformChangedEvent &event);
  void OnMaterialChanged(MaterialChangedEvent &event);

  /**
   * 将ECS实体转换为RenderableEntity并加入渲染队列
   * @param entity 需要添加的ECS实体
   */
  bool AddToRenderQueue(Entity entity);

  /**
   * 从渲染队列移除实体
   * @param entity 需要移除的ECS实体
   */
  void RemoveFromRenderQueue(Entity entity);

  /**
   * 内部方法：更新实体在渲染队列中的变换或材质
   * @param entity 需要更新的ECS实体
   */
  void UpdateRenderableEntity(Entity entity);

 private:
  //=== 数据成员 ===//
  SceneRegistry &m_Registry;                     // SceneCore的ECS注册表引用
  std::unordered_set<Entity> m_PendingEntities;  // 当前帧新创建的实体列表，下一帧AddToRenderQueue
  std::vector<std::shared_ptr<RenderableItem>> m_RenderQueue;  // 当前帧的渲染队列

  // 事件订阅集合（通过RAII自动取消订阅）
  SubscriptionGroup m_EventSubscriptions;

  // 快速查找表：实体ID到渲染队列下标的映射（用于高效更新）
  std::unordered_map<Entity, size_t> m_EntityToIndexMap;
};
}  // namespace mite

#endif