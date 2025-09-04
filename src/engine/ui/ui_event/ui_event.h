#ifndef MITE_UI_EVENT_H
#define MITE_UI_EVENT_H

#include "event/event.h"
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
  virtual uint64_t GetSourceWidgetID() const
  {
    return 0;
  }
};

}  // namespace mite

#endif  // MITE_UI_EVENT_H
