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
  SceneObserver(std::weak_ptr<Scene> scene);
  ~SceneObserver();

  /**
   * @brief 开始观察场景变更
   */
  void BeginObservation();

  /**
   * @brief 结束观察并处理所有待处理变更
   * @param dispatcher 事件分发器，用于发送实体级变更事件
   */
  void EndObservationAndEmitEvents(EventDispatcher &dispatcher);

  /**
   * @brief 停止观察并清空缓存
   */
  void Clear();

 private:

  /**
   * @brief 标记实体为已修改
   * @param entity 被修改的实体
   * @note 会通知ComponentSystemManager检查组件变更
   */
  bool MarkEntityModified(Entity entity);

  /**
   * @brief 处理实体创建
   */
  bool OnEntityCreated(Entity entity);

  /**
   * @brief 处理实体销毁
   */
  bool OnEntityDestroyed(Entity entity);

 private:
  std::weak_ptr<Scene> m_Scene;  // 场景引用
  std::vector<Entity> m_CreatedEntities;
  std::vector<Entity> m_DestroyedEntities;
  std::vector<Entity> m_ModifiedEntities;
  bool m_IsObserving = false;
};
};  // namespace mite

#endif
