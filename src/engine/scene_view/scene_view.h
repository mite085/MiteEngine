#ifndef MITE_SCENE_VIEW_H
#define MITE_SCENE_VIEW_H
#include "basic_data/camera.h"
#include "render_queue.h"
#include "renderable_item_builder.h"
#include "scene_node.h"

namespace mite {

/**
 * @brief 场景视图管理器
 * @note 职责：负责渲染数据的收集、组织和交付，作为SceneGraph与Renderer之间的桥梁
 * @note 单一职责：专注于渲染数据管理，不涉及ECS事件监听
 */
class SceneView {
 public:
  /**
   * @brief 构造函数
   * @param sceneGraph 场景图引用
   * @param registry 场景注册表引用（用于构建器创建）
   */
  SceneView(SceneGraph *sceneGraph, SceneRegistry &registry);

  /**
   * @brief 析构函数
   */
  ~SceneView();
  // ---- 核心接口 ----

  /**
   * @brief 更新场景视图（每帧调用）
   * @param camera 摄像机（用于视锥体剔除）
   * @note 每帧完全重建渲染队列
   */
  void Update(Camera *camera);

  /**
   * @brief 强制重建渲染队列（手动调用）
   * @param camera 摄像机
   */
  void Rebuild(Camera *camera);

  /**
   * @brief 获取渲染队列
   * @return 渲染队列的共享指针
   */
  std::shared_ptr<RenderQueue> GetRenderQueue() const;
  // ---- 配置接口 ----

  /**
   * @brief 设置可见性掩码
   * @param mask 可见性掩码（用于过滤渲染对象）
   */
  void SetVisibilityMask(uint32_t mask);

  /**
   * @brief 获取当前可见性掩码
   * @return 可见性掩码
   */
  uint32_t GetVisibilityMask() const;

  /**
   * @brief 设置自定义渲染过滤器
   * @param filterFunc 过滤函数（返回true表示包含该节点）
   */
  void SetCustomFilter(std::function<bool(SceneNode *)> filterFunc);
  // ---- 统计信息 ----

  /**
   * @brief 获取可见节点数量
   * @return 当前帧的可见节点数量
   */
  size_t GetVisibleNodeCount() const;

  /**
   * @brief 获取渲染项数量
   * @return 当前帧的渲染项数量
   */
  size_t GetRenderItemCount() const;

  /**
   * @brief 获取上次更新耗时（毫秒）
   * @return 更新耗时
   */
  float GetLastUpdateTime() const;

 private:
  /**
   * @brief 执行可见性查询和渲染项构建
   * @param camera 摄像机
   */
  void ProcessVisibility(Camera *camera);

  /**
   * @brief 应用自定义过滤器
   * @param nodes 输入节点列表
   * @return 过滤后的节点列表
   */
  std::vector<SceneNode *> ApplyCustomFilter(const std::vector<SceneNode *> &nodes);
  SceneGraph *m_sceneGraph;                          // 场景图引用（不拥有所有权）
  std::unique_ptr<RenderableItemBuilder> m_builder;  // 渲染项构建器
  std::shared_ptr<RenderQueue> m_renderQueue;        // 渲染队列

  uint32_t m_visibilityMask;                            // 可见性掩码
  std::function<bool(SceneNode *)> m_customFilterFunc;  // 自定义过滤器

  // 统计信息
  size_t m_lastVisibleNodeCount;  // 上次可见节点数量
  size_t m_lastRenderItemCount;   // 上次渲染项数量
  float m_lastUpdateTime;         // 上次更新耗时（毫秒）

  // 禁用拷贝构造和赋值
  SceneView(const SceneView &) = delete;
  SceneView &operator=(const SceneView &) = delete;

  // 日志器
  Logger m_logger;
};

//class SceneView {
// public:
//  /**
//   * 构造函数：需要传入SceneCore的ECS注册表和事件总线
//   * @param registry EnTT注册表引用（由SceneCore持有）
//   * @param eventBus 事件总线引用（用于订阅场景变更事件）
//   */
//  explicit SceneView(SceneRegistry &registry);
//  ~SceneView();
//
//  /**
//   * 每帧更新渲染数据（在Application主循环中调用）
//   * 1. 处理ECS事件（实体增删改）
//   * 2. 更新内部渲染队列
//   */
//  void Update();
//
//  /**
//   * 获取当前帧的可渲染实体列表（供Renderer模块调用）
//   * @return 只读的渲染队列引用，避免数据拷贝
//   */
//  const std::vector<std::shared_ptr<RenderableItem>> &GetRenderQueue() const;
//
// private:
//  //=== 事件处理函数（订阅SceneCore的事件） ===//
//  bool OnEntityCreated(EntityCreatedEvent &event);
//  bool OnEntityDestroyed(EntityDestroyedEvent &event);
//  bool OnTransformChanged(TransformChangedEvent &event);
//  bool OnMaterialChanged(MaterialChangedEvent &event);
//
//  /**
//   * 将ECS实体转换为RenderableEntity并加入渲染队列
//   * @param entity 需要添加的ECS实体
//   */
//  bool AddToRenderQueue(Entity entity);
//
//  /**
//   * 从渲染队列移除实体
//   * @param entity 需要移除的ECS实体
//   */
//  void RemoveFromRenderQueue(Entity entity);
//
//  /**
//   * 内部方法：更新实体在渲染队列中的变换或材质
//   * @param entity 需要更新的ECS实体
//   */
//  void UpdateRenderableEntity(Entity entity);
//
// private:
//  //=== 数据成员 ===//
//  SceneRegistry &m_Registry;                     // SceneCore的ECS注册表引用
//  std::unordered_set<Entity> m_PendingEntities;  // 当前帧新创建的实体列表，下一帧AddToRenderQueue
//  std::vector<std::shared_ptr<RenderableItem>> m_RenderQueue;  // 当前帧的渲染队列
//
//  // 事件订阅集合（通过RAII自动取消订阅）
//  SubscriptionGroup m_EventSubscriptions;
//
//  // 快速查找表：实体ID到渲染队列下标的映射（用于高效更新）
//  std::unordered_map<Entity, size_t> m_EntityToIndexMap;
//};
}  // namespace mite

#endif