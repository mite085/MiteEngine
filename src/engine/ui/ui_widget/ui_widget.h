#ifndef MITE_UI_WIDGET_H
#define MITE_UI_WIDGET_H

#include "ui_core/ui_style.h"
#include "ui_event/ui_event.h"
#include "ui_event/ui_events_interaction.h"

namespace mite {

/**
 * @brief UI控件基类，所有UI控件的抽象基类
 */
class UIWidget {
 public:
  explicit UIWidget(const std::string &name = "");
  virtual ~UIWidget();

  /**
   * @brief 获取控件唯一ID
   */
  UUID GetID() const
  {
    return m_ID;
  }

  /**
   * @brief 获取控件名称
   */
  const std::string &GetName() const
  {
    return m_Name;
  }

  /**
   * @brief 设置控件名称
   */
  void SetName(const std::string &name)
  {
    m_Name = name;
  }

  /**
   * @brief 获取控件位置
   */
  glm::vec2 GetPosition() const
  {
    return m_Position;
  }

  /**
   * @brief 设置控件位置
   */
  virtual void SetPosition(const glm::vec2 &position);

  /**
   * @brief 获取控件尺寸
   */
  glm::vec2 GetSize() const
  {
    return m_Size;
  }

  /**
   * @brief 设置控件尺寸
   */
  virtual void SetSize(const glm::vec2 &size);

  /**
   * @brief 获取控件可见性
   */
  bool IsVisible() const
  {
    return m_Visible;
  }

  /**
   * @brief 设置控件可见性
   */
  virtual void SetVisible(bool visible);

  /**
   * @brief 获取控件是否启用
   */
  bool IsEnabled() const
  {
    return m_Enabled;
  }

  /**
   * @brief 设置控件是否启用
   */
  virtual void SetEnabled(bool enabled);

  /**
   * @brief 获取控件样式
   */
  std::shared_ptr<UIStyle> GetStyle() const
  {
    return m_Style;
  }

  /**
   * @brief 设置控件样式
   */
  virtual void SetStyle(std::shared_ptr<UIStyle> style);

  /**
   * @brief 检查点是否在控件区域内
   */
  virtual bool ContainsPoint(const glm::vec2 &point) const;

  /**
   * @brief 处理鼠标进入事件
   */
  virtual void OnMouseEnter(const MouseEnterEvent &event);

  /**
   * @brief 处理鼠标离开事件
   */
  virtual void OnMouseLeave(const MouseLeaveEvent &event);

  /**
   * @brief 更新控件状态
   */
  virtual void Update(float deltaTime);

  /**
   * @brief 渲染控件
   */
  virtual void Render() = 0;

  /**
   * @brief 发布UI事件
   */
  template<typename EventType, typename... Args> void PublishEvent(Args &&...args)
  {
    if (m_EventBus) {
      auto event = std::make_shared<EventType>(std::forward<Args>(args)...);
      m_EventBus->Publish(event);
    }
  }

 protected:
  UUID m_ID;
  std::string m_Name;
  glm::vec2 m_Position;
  glm::vec2 m_Size;
  bool m_Visible;
  bool m_Enabled;
  std::shared_ptr<UIStyle> m_Style;
};

}  // namespace mite

#endif  // MITE_UI_WIDGET_H
