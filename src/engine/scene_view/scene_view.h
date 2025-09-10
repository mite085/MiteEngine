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
  SceneView();

  /**
   * @brief 析构函数
   */
  ~SceneView();
  // ----  ----
  // ==================== 核心接口 ====================
  /**
   * @brief 更新场景视图（每帧调用）
   * @note 每帧完全重建渲染队列
   */
  void Update(SceneRegistry &registry, std::vector<SceneNode *> visibleNodes);

  /**
   * @brief 强制重建渲染队列（手动调用）
   */
  void Rebuild(SceneRegistry &registry, std::vector<SceneNode *> visibleNodes);

  /**
   * @brief 获取渲染队列
   * @return 渲染队列的共享指针
   */
  std::shared_ptr<RenderQueue> GetRenderQueue() const;
  // ----  ----
  // ==================== 配置接口 ====================
  /**
   * @brief 设置自定义渲染过滤器（对visibleNodes执行进一步筛选）
   * @param filterFunc 过滤函数（返回true表示包含该节点）
   */
  void SetCustomFilter(std::function<bool(SceneNode *)> filterFunc);
  // ----  ----
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

  /**
   * @brief 应用自定义过滤器
   * @param nodes 输入节点列表
   * @return 过滤后的节点列表
   */
  std::vector<SceneNode *> ApplyCustomFilter(const std::vector<SceneNode *> &nodes);


  std::unique_ptr<RenderableItemBuilder> m_Builder;  // 渲染Item构建器
  std::shared_ptr<RenderQueue> m_RenderQueue;        // 渲染队列

  std::function<bool(SceneNode *)> m_CustomFilterFunc;  // 自定义过滤器

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