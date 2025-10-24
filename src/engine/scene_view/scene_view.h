#ifndef MITE_SCENE_VIEW_H
#define MITE_SCENE_VIEW_H
#include "basic_event/instance_event.h"
#include "basic_event/render_event.h"
#include "basic_instance/camera_instance.h"
#include "render_queue.h"
#include "renderable_item_builder.h"
#include "scene_core/scene_core.h"
#include "scene_graph.h"

namespace mite {
/**
 * @brief 场景视图管理器
 * @note 职责：负责渲染数据的收集、组织和交付，作为SceneGraph与Renderer之间的桥梁
 * @note 单一职责：专注于渲染数据管理，不涉及ECS事件监听
 */
class SceneView {
 public:
  SceneView(SceneCore &sceneCore, SceneGraph &sceneGraph);
  ~SceneView();
  void Initialize();

  // ==================== 数据接口 ====================
  /**
   * @brief 更新场景视图（每帧调用）
   * @note 每帧完全重建渲染队列
   */
  void Update();
  /**
   * @brief 获取渲染队列
   * @return 渲染队列的共享指针
   */
  std::shared_ptr<RenderQueue> GetRenderQueue() const;

  // ==================== 交互接口 ====================
  /**
   * @brief 获取当前摄像机实例
   */
  std::shared_ptr<CameraInstance> GetCameraInstance() const { return m_CameraInstance; }
  Entity GetCameraEntity() const { return m_CameraEntity; }
  /**
   * @brief 选择场景对象，设置模型矩阵（世界坐标）
   * （若当前相机为选中对象，则并非正常选中状态，IsPicked()返回false）
   */
  bool Pick(glm::vec2 screenPosUV);
  bool IsPicked() const { return m_PickedEntity.IsValid() && m_PickedEntity != m_CameraEntity; }
  void SetPickedWorldTransform(const Transform &worldTransform);
  Transform GetPickedWorldTransform() const;
  /**
   * @brief 设定当前相机变换矩阵
   */
  void SetCameraWorldTransform(const Transform &worldTransform);
  void SetCameraZoom(float zoom);

  // ==================== 统计信息 ====================
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

 private:
  // ==================== 内部方法 ====================
  // 执行渲染Item构建
  void ProcessVisibility(std::vector<SceneNode *> visibleNodes);

  // 事件订阅
  void OnViewportResize(ViewportResizeEvent &event);               // 修改Camera的宽高比
  void OnViewportPicked(ViewportPickedEvent &event);               // 更新m_PickedEntity状态
  void OnViewportCameraUpdated(ViewportCameraUpdateEvent &event);  // 更新相机状态
  void OnViewportPickedUpdated(ViewportPickedUpdateEvent &event);  // 更新picked状态
  void OnSceneNodeSelected(SceneNodeSelectedEvent &event);		   // 选择场景节点

  // 依赖注入
  SceneCore &m_SceneCore;
  SceneGraph &m_SceneGraph;

  // 成员变量
  std::unique_ptr<RenderableItemBuilder> m_Builder;  // 渲染Item构建器
  std::shared_ptr<RenderQueue> m_RenderQueue;        // 渲染队列
  Entity m_PickedEntity;                             // 拾取的Entity
  Entity m_CameraEntity;                             // 关联的摄像机Entity
  std::shared_ptr<CameraInstance> m_CameraInstance;  // 关联的摄像机实例

  // 统计信息
  size_t m_LastVisibleNodeCount;  // 上次可见节点数量
  size_t m_LastRenderItemCount;   // 上次渲染Item数量

  // 禁用拷贝构造和赋值
  SceneView(const SceneView &) = delete;
  SceneView &operator=(const SceneView &) = delete;

  // 日志器
  Logger m_Logger;

  // 事件订阅
  SubscriptionGroup m_EventSubscriptions;
};
}  // namespace mite

#endif