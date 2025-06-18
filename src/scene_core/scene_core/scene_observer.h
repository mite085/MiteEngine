#ifndef MITE_SCENE_OBSERVER
#define MITE_SCENE_OBSERVER

#include "component_id.h"
#include "scene_registry.h"

namespace mite {
// 场景变更类型枚举
enum class SceneChangeType {
  VOID_TYPE,          // 无Change,遇到时忽略
  ENTITY_CREATED,     // 实体创建
  ENTITY_DESTROYED,   // 实体销毁
  COMPONENT_ADDED,    // 组件添加
  COMPONENT_REMOVED,  // 组件移除
  COMPONENT_CHANGED,  // 组件修改
  PARENT_CHANGED,     // 父节点变化
  TAG_CHANGED,        // 标签变化
  SCENE_LOADED,       // 场景加载
  SCENE_CLEARED       // 场景清空
};

// 场景变更事件数据结构
class SceneChangeEvent:public Event {
 public:
  SceneChangeType changeType = SceneChangeType::VOID_TYPE;  // 变更类型
  Entity entity;                                            // 关联的实体
  ComponentID componentType;      // 关联的组件类型ID (如果是组件相关变更)
  std::shared_ptr<void> oldData;  // 变更前的数据 (可选)
  std::shared_ptr<void> newData;  // 变更后的数据 (可选)
};

// 场景观察者回调函数类型
using SceneObserverCallback = std::function<void(const SceneChangeEvent &)>;

/**
 * @class SceneObserver
 * @brief 场景变更观察者，跟踪场景中的各种变更并通知注册的回调
 *
 * 这个类完全基于封装的SceneRegistry和Entity工作，不直接接触entt内部实现
 */
class SceneObserver {
 public:
  /**
   * @brief 构造函数
   * @param registry 要观察的场景注册表
   */
  explicit SceneObserver(SceneRegistry &registry);

  ~SceneObserver();

  /**
   * @brief 注册变更回调
   * @param callback 变更发生时调用的回调函数
   * @return 回调ID，可用于取消注册
   */
  size_t RegisterCallback(const SceneObserverCallback &callback);

  /**
   * @brief 取消注册变更回调
   * @param callbackId 要移除的回调ID
   */
  void UnregisterCallback(size_t callbackId);

  /**
   * @brief 开始跟踪场景变更
   * 调用此方法后，SceneObserver将开始记录所有场景变更
   */
  void StartTracking();

  /**
   * @brief 停止跟踪场景变更
   * 调用此方法后，SceneObserver将停止记录场景变更
   */
  void StopTracking();

  /**
   * @brief 获取自上次检查以来的所有变更
   * @return 变更事件列表
   */
  std::vector<SceneChangeEvent> FlushChanges();

  /**
   * @brief 检查实体是否在本次跟踪会话中被修改过
   * @param entity 要检查的实体
   * @return 如果实体被修改过则返回true
   */
  bool IsEntityDirty(Entity entity) const;

 private:
  // 初始化所有必要的回调
  void SetupCallbacks();

  // 清理所有回调
  void CleanupCallbacks();

  // 通知所有注册的回调
  void NotifyCallbacks(const SceneChangeEvent &event);

  // 处理实体创建
  void OnEntityCreated(Entity entity);

  // 处理实体销毁
  void OnEntityDestroyed(Entity entity);

  // 处理组件添加
  template<typename T> void OnComponentAdded(Entity entity, T &component);

  // 处理组件移除
  template<typename T> void OnComponentRemoved(Entity entity, T &component);

  // 处理组件修改
  template<typename T> void OnComponentChanged(Entity entity, T &component);

 private:
  SceneRegistry &m_Registry;  // 观察的场景注册表引用

  std::unordered_map<size_t, SceneObserverCallback> m_Callbacks;  // 注册的回调函数
  size_t m_NextCallbackId = 0;                                    // 下一个回调ID

  std::vector<SceneChangeEvent> m_ChangeEvents;  // 收集的变更事件
  std::unordered_set<Entity> m_DirtyEntities;    // 脏实体集合

  bool m_IsTracking = false;  // 是否正在跟踪变更

  // 以下是与SceneRegistry的连接句柄
  std::function<void(Entity)> m_EntityCreatedCallback;
  std::function<void(Entity)> m_EntityDestroyedCallback;

  // 组件回调存储
  struct ComponentCallbackData {
    std::function<void(Entity, void *)> added;
    std::function<void(Entity, void *)> removed;
    std::function<void(Entity, void *)> changed;
  };

  std::unordered_map<ComponentID, ComponentCallbackData> m_ComponentCallbacks;
};
};  // namespace mite

#endif
