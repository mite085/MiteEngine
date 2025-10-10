#ifndef MITE_SCENE_VIEW_H
#define MITE_SCENE_VIEW_H
#include "basic_instance/camera_instance.h"
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
  SceneView();
  ~SceneView();

  // ==================== 核心接口 ====================
  /**
   * @brief 设定关联的摄像机实例（执行渲染之前设定一次即可）
   */
  void SetCamera(const std::shared_ptr<Camera> &camera);
  // 获取当前关联的摄像机实例
  std::shared_ptr<CameraInstance>GetCameraInstance() const { return m_CameraInstance; }
  /**
   * @brief 更新场景视图（每帧调用）
   * @note 每帧完全重建渲染队列
   */
  void Update(SceneRegistry &registry,
              Transform cameraTransform,
              std::vector<SceneNode *> visibleNodes);

  /**
   * @brief 获取渲染队列
   * @return 渲染队列的共享指针
   */
  std::shared_ptr<RenderQueue> GetRenderQueue() const;

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

  /**
   * @brief 获取上次更新耗时（毫秒）
   * @return 更新耗时
   */
  float GetLastUpdateTime() const;

 private:
  // ==================== 内部方法 ====================
  /**
   * @brief 执行渲染Item构建
   */
  void ProcessVisibility(SceneRegistry &registry, std::vector<SceneNode *> visibleNodes);


  // 成员变量
  std::unique_ptr<RenderableItemBuilder> m_Builder;  // 渲染Item构建器
  std::shared_ptr<RenderQueue> m_RenderQueue;        // 渲染队列
  std::shared_ptr<CameraInstance> m_CameraInstance;  // 关联的摄像机实例

  // 统计信息
  size_t m_LastVisibleNodeCount;  // 上次可见节点数量
  size_t m_LastRenderItemCount;   // 上次渲染Item数量
  float m_LastUpdateTime;         // 上次更新耗时（毫秒）

  // 禁用拷贝构造和赋值
  SceneView(const SceneView &) = delete;
  SceneView &operator=(const SceneView &) = delete;

  // 日志器
  Logger m_Logger;
};

}  // namespace mite

#endif