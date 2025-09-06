#ifndef MITE_UI_PANEL_H
#define MITE_UI_PANEL_H

#include "ui_widget.h"

namespace mite {

/**
 * @brief UI面板基类，用于管理一组控件
 */
class UIPanel : public UIWidget {
 public:
  explicit UIPanel(const std::string &name = "");
  virtual ~UIPanel();

  /**
   * @brief 添加子控件
   */
  virtual void AddWidget(std::shared_ptr<UIWidget> widget);

  /**
   * @brief 移除子控件
   */
  virtual void RemoveWidget(UUID widgetId);

  /**
   * @brief 获取子控件
   */
  std::shared_ptr<UIWidget> GetWidget(UUID widgetId) const;

  /**
   * @brief 获取所有子控件
   */
  const std::vector<std::shared_ptr<UIWidget>> &GetWidgets() const
  {
    return m_Widgets;
  }

  /**
   * @brief 清空所有子控件
   */
  virtual void ClearWidgets();

  /**
   * @brief 订阅事件
   */
  template<typename EventType> void SubscribeEvent(std::function<void(const EventType &)> callback)
  {
    if (m_EventBus) {
      m_EventBus->Subscribe<EventType>([callback](const std::shared_ptr<Event> &event) {
        if (auto typedEvent = std::dynamic_pointer_cast<EventType>(event)) {
          callback(*typedEvent);
        }
      });
    }
  }

  /**
   * @brief 更新面板状态
   */
  virtual void Update(float deltaTime) override;

  /**
   * @brief 渲染面板及其子控件
   */
  virtual void Render() override;

  /**
   * @brief 处理布局计算
   */
  virtual void CalculateLayout();

  /**
   * @brief 获取面板标题
   */
  const std::string &GetTitle() const
  {
    return m_Title;
  }

  /**
   * @brief 设置面板标题
   */
  void SetTitle(const std::string &title)
  {
    m_Title = title;
  }

  /**
   * @brief 获取面板是否可拖动
   */
  bool IsDraggable() const
  {
    return m_Draggable;
  }

  /**
   * @brief 设置面板是否可拖动
   */
  void SetDraggable(bool draggable)
  {
    m_Draggable = draggable;
  }

  /**
   * @brief 获取面板是否可调整大小
   */
  bool IsResizable() const
  {
    return m_Resizable;
  }

  /**
   * @brief 设置面板是否可调整大小
   */
  void SetResizable(bool resizable)
  {
    m_Resizable = resizable;
  }

 protected:
  std::string m_Title;
  bool m_Draggable;
  bool m_Resizable;
  std::vector<std::shared_ptr<UIWidget>> m_Widgets;
  std::unordered_map<UUID, std::shared_ptr<UIWidget>> m_WidgetMap;
};

}  // namespace mite

#endif  // MITE_UI_PANEL_H
