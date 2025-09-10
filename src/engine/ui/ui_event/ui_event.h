#ifndef MITE_UI_EVENT_H
#define MITE_UI_EVENT_H

#include "event/event.h"
#include "uuid/mite_uuid.h"
#include <glm/glm.hpp>

namespace mite {

/**
 * @brief UI事件基类
 */
class UIEvent : public Event {
 public:
  virtual ~UIEvent() = default;

  /**
   * @brief 获取事件源控件ID
   */
  virtual UUID GetSourceWidgetID() const
  {
    return {};
  }
};

}  // namespace mite

#endif  // MITE_UI_EVENT_H
