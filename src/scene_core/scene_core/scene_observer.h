#ifndef MITE_SCENE_OBSERVER
#define MITE_SCENE_OBSERVER

#include "component_id.h"
#include "scene_registry.h"
#include "scene_event.h"

namespace mite {
// 前向声明
class Scene;
/**
 * @brief 场景观察者类
 * 仅负责实体层级的变更跟踪，组件变更委托给ComponentSystemManager
 */
class SceneObserver {
 public:
  SceneObserver(SceneRegistry &registry);
  ~SceneObserver();

  /**
   * @brief 开始观察场景变更
   */
  void BeginObservation();

  /**
   * @brief 结束观察并处理所有待处理变更
   */
  void EndObservation();

  /**
   * @brief 停止观察并清空缓存
   */
  void Clear();

 private:

  /**
   * @brief 标记实体为已修改
   */
  void MarkEntityModified(EntityParentChangedEvent &e);

  /**
   * @brief 处理实体创建
   */
  void OnEntityCreated(EntityCreatedEvent &e);

  /**
   * @brief 处理实体销毁，仅在实体销毁之前处理。
   */
  void OnEntityPreDestroyed(EntityPreDestroyedEvent &e);


 private:
  SceneRegistry& m_Registry;  // 场景注册信息引用
  std::vector<Entity> m_CreatedEntities;
  std::vector<Entity> m_DestroyedEntities;
  std::vector<Entity> m_ModifiedEntities;
  bool m_IsObserving = false;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
};
};  // namespace mite

#endif
