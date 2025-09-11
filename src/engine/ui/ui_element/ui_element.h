#ifndef MITE_UI_ELEMENT_H
#define MITE_UI_ELEMENT_H

#include "input/input_event.h"
#include "ui_core/ui_style.h"
#include "ui_event/ui_events_interaction.h"
#include "ui_event/ui_events_layout.h"
#include "ui_event/ui_events_lifecycle.h"

namespace mite {
/**
 * @brief UI元素基类，所有UI可交互元素的抽象基类
 *
 * UIElement提供了UI系统中最基础的属性和方法抽象，
 * 作为UIWidget和其他UI组件的共同基类
 */
class UIElement {
 public:
  explicit UIElement(const std::string &name);
  virtual ~UIElement();

  // 禁止拷贝和赋值
  UIElement(const UIElement &) = delete;
  UIElement &operator=(const UIElement &) = delete;

  // ==================== 每帧执行，核心函数，需要子类重写 ====================
  /**
   * @brief 更新元素状态
   */
  virtual void Update(float deltaTime) = 0;

  /**
   * @brief 渲染元素
   */
  virtual void Render() = 0;

  // ==================== 基本数据接口（无需子类重写） ====================
  /**
   * @brief 获取元素唯一ID
   */
  UUID GetID() const;

  /**
   * @brief 获取元素名称
   */
  const std::string &GetName() const;

  /**
   * @brief 设置元素名称
   */
  virtual void SetName(const std::string &name);

  /**
   * @brief 获取元素位置
   */
  glm::vec2 GetPosition() const;

  /**
   * @brief 设置元素位置
   */
  virtual void SetPosition(const glm::vec2 &position);

  /**
   * @brief 获取元素尺寸
   */
  glm::vec2 GetSize() const;

  /**
   * @brief 设置元素尺寸
   */
  virtual void SetSize(const glm::vec2 &size);

  /**
   * @brief 获取元素可见性
   */
  bool IsVisible() const;

  /**
   * @brief 设置元素可见性
   */
  virtual void SetVisible(bool visible);

  /**
   * @brief 获取元素是否启用
   */
  bool IsEnabled() const;

  /**
   * @brief 设置元素是否启用
   */
  virtual void SetEnabled(bool enabled);

  /**
   * @brief 获取元素边界矩形
   */
  virtual glm::vec4 GetBounds() const;

  // ======= 事件相关（主要处理鼠标悬停高亮，与鼠标点击效果，无需子类重写）========
  /**
   * @brief 处理鼠标进入事件
   */
  virtual void OnMouseEnter(const MouseEnterEvent &event);

  /**
   * @brief 处理鼠标离开事件
   */
  virtual void OnMouseLeave(const MouseLeaveEvent &event);

  /**
   * @brief 检查点是否在元素区域内
   */
  virtual bool ContainsPoint(const glm::vec2 &point) const;

  /**
   * @brief 设置元素悬停状态（悬停状态表示鼠标在控件区域内但未按下按钮）
   *
   * 事件逻辑：
   * 鼠标进入控件区域 → OnMouseEnter → SetHovered(true) → 发布悬停开始事件
   * 鼠标在控件内移动 → 保持悬停状态
   * 鼠标离开控件区域 → OnMouseLeave → SetHovered(false) → 发布悬停结束事件
   */
  virtual void SetHovered(bool hovered);
  /**
   * @brief 获取元素悬停状态
   */
  bool IsHovered() const;

  /**
   * @brief 设置元素焦点状态（当UI元素成为当前用户输入的主要接收者时，元素获得焦点状态）
   *
   * 事件逻辑：
   * 鼠标移动 → 检测悬停元素 → 发布悬停事件 → 更新视觉反馈
   * 鼠标点击 → 设置焦点元素 → 发布焦点事件 → 开始接收键盘输入
   * 键盘输入 → 只有焦点元素处理 → 执行相应操作
   */
  virtual void SetFocused(bool focused);
  /**
   * @brief 获取元素焦点状态
   */
  bool IsFocused() const;

 protected:
  UUID m_ID;
  std::string m_Name;
  glm::vec2 m_Position;
  glm::vec2 m_Size;
  bool m_Visible;
  bool m_Enabled;
  bool m_Focused;
  bool m_Hovered;
};
}  // namespace mite

#endif  // MITE_UI_ELEMENT_H
