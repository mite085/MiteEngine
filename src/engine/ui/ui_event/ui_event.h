#ifndef MITE_UI_EVENT_H
#define MITE_UI_EVENT_H
#include "ui_panel/ui_panel.h"
#include "subscription_group.h"
#include "uuid/mite_uuid.h"
#include <glm/glm.hpp>

namespace mite {

/**
 * @brief UI事件基类
 */
class UIEvent : public Event {
 public:
  UIEvent(std::shared_ptr<UIPanel> panel) : m_Panel(panel) {}
  virtual ~UIEvent() = default;

  /**
   * @brief 获取事件源控件ID
   */
  std::shared_ptr<UIPanel> GetPanel() const
  {
    return m_Panel;
  }

 protected:
  std::shared_ptr<UIPanel> m_Panel;
};

}  // namespace mite

#endif  // MITE_UI_EVENT_H
