#ifndef MITE_UI_OVERLAY_H
#define MITE_UI_OVERLAY_H

#include "ui_core/ui_render.h"
#include "ui_core/ui_render_props.h"

namespace mite {
/**
 * @brief 覆盖层渲染上下文
 *
 * 提供Overlay渲染所需的所有上下文信息
 */
struct OverlayContext {
  // 视口信息
  glm::vec2 viewportPos;   // 视口左上角屏幕坐标
  glm::vec2 viewportSize;  // 视口尺寸
  glm::vec2 contentPos;    // 内容区域左上角坐标
  glm::vec2 contentSize;   // 内容区域尺寸

  // 相机和变换信息
  glm::mat4 viewMatrix;        // 视图矩阵
  glm::mat4 projectionMatrix;  // 投影矩阵
  glm::mat4 modelMatrix;       // 模型矩阵（可选）

  // 输入状态
  bool isViewportHovered;  // 视口是否被悬停
  bool isViewportFocused;  // 视口是否获得焦点
  glm::vec2 mousePos;      // 鼠标位置（视口相对坐标）

  // 帧状态
  float deltaTime;  // 帧间隔时间

  OverlayContext()
      : viewportPos(0, 0),
        viewportSize(0, 0),
        contentPos(0, 0),
        contentSize(0, 0),
        viewMatrix(1.0f),
        projectionMatrix(1.0f),
        modelMatrix(1.0f),
        isViewportHovered(false),
        isViewportFocused(false),
        mousePos(0, 0),
        deltaTime(0)
  {
  }
};
/**
 * @brief UI覆盖层抽象基类 - 轻量级叠加渲染组件
 *
 * 设计原则：
 * 1. 无状态管理：不管理自己的窗口状态
 * 2. 依赖注入：渲染时传入目标上下文
 * 3. 事件透传：通过上下文处理输入事件
 * 4. 最小依赖：仅包含必要的渲染逻辑
 */
class UIOverlay {
 public:
  explicit UIOverlay() : m_Renderer(UIRender::Get()) {}
  virtual ~UIOverlay() = default;

  // 禁止拷贝
  UIOverlay(const UIOverlay &) = delete;
  UIOverlay &operator=(const UIOverlay &) = delete;

  // ==================== 核心接口 ====================
  /**
   * @brief 更新覆盖层状态
   * @param deltaTime 帧间隔时间
   */
  virtual void Update(float deltaTime) = 0;

  /**
   * @brief 渲染覆盖层内容
   * @param context 渲染上下文信息
   */
  virtual void Render(const OverlayContext &context) = 0;

  // ==================== 基础属性访问 ====================
  bool IsEnabled() const { return m_Enabled; }
  void SetEnabled(bool enabled) { m_Enabled = enabled; }
  bool IsVisible() const { return m_Visible; }
  void SetVisible(bool visible) { m_Visible = visible; }

  // ==================== 层级控制 ====================
  int GetRenderOrder() const { return m_RenderOrder; }
  void SetRenderOrder(int order) { m_RenderOrder = order; }

 protected:
  bool m_Enabled = true;
  bool m_Visible = true;
  int m_RenderOrder = 0;  // 渲染顺序，数值小的先渲染
  UIRender &m_Renderer;
};

}  // namespace mite

#endif  // MITE_UI_OVERLAY_H
