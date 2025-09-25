#ifndef MITE_UI_PANEL_H
#define MITE_UI_PANEL_H

#include "ui_core/ui_render.h"
#include "ui_core/ui_render_props.h"

namespace mite {

/**
 * @brief UI面板抽象基类 - 基于属性数据驱动
 *
 * 设计原则：
 * 1. 数据驱动：通过PanelProps属性结构控制渲染
 * 2. 属性管理：面板管理自己的属性状态
 * 3. 渲染解耦：通过UIRender接口渲染
 */
class UIPanel {
 public:
  explicit UIPanel(const std::string &name);
  virtual ~UIPanel() = default;
  // 禁止拷贝
  UIPanel(const UIPanel &) = delete;
  UIPanel &operator=(const UIPanel &) = delete;
  // ==================== 核心接口 ====================
  /**
   * @brief 更新面板状态
   */
  virtual void Update(float deltaTime) = 0;
  /**
   * @brief 渲染面板内容（子类无需关心Begin、End和Visible）
   */
  virtual void Render() = 0;
  /**
   * @brief 包装了Begin和End逻辑，以及Visible判断的Render操作
   */
  void RenderPanel();
  // ==================== 基础属性访问 ====================
  const std::string &GetName() const
  {
    return m_PanelProps.fallbackText;
  }
  bool IsVisible() const
  {
    return m_PanelProps.visible;
  }
  void SetVisible(bool visible)
  {
    m_PanelProps.visible = visible;
  }
  bool IsEnabled() const
  {
    return m_PanelProps.enabled;
  }
  void SetEnabled(bool enabled)
  {
    m_PanelProps.enabled = enabled;
  }
  // ==================== PanelProps访问器 ====================
  PanelProps &GetPanelProps()
  {
    return m_PanelProps;
  }
  const PanelProps &GetPanelProps() const
  {
    return m_PanelProps;
  }

  void SetPanelProps(const PanelProps &props)
  {
    m_PanelProps = props;
  }
  // ==================== 便捷属性设置 ====================
  void SetMovable(bool movable)
  {
    m_PanelProps.movable = movable;
  }
  void SetResizable(bool resizable)
  {
    m_PanelProps.resizable = resizable;
  }
  void SetScrollable(bool scrollable)
  {
    m_PanelProps.scrollable = scrollable;
  }
  void SetMinSize(const glm::vec2 &size)
  {
    m_PanelProps.minSize = size;
  }
  void SetMaxSize(const glm::vec2 &size)
  {
    m_PanelProps.maxSize = size;
  }

 protected:
  // ==================== 受保护的渲染接口 ====================
  /**
   * @brief 获取渲染接口引用
   */
  UIRender &GetRenderer()
  {
    return m_Renderer;
  }
  /**
   * @brief 获取内容区域可用尺寸
   */
  glm::vec2 GetContentRegionAvail()
  {
    return m_Renderer.GetContentRegionAvail();
  }
  // ==================== 面板属性 ====================
  PanelProps m_PanelProps;

 private:
  UIRender &m_Renderer;
};
}  // namespace mite

#endif  // MITE_UI_PANEL_H
