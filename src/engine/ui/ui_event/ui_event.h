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
  UIEvent(UUID elementId) : m_ElementId(elementId){}
  virtual ~UIEvent() = default;

  /**
   * @brief 获取事件源控件ID
   */
  virtual UUID GetSourceElementID() const
  {
    return m_ElementId;
  }

 protected:
  UUID m_ElementId;
};

}  // namespace mite

#endif  // MITE_UI_EVENT_H
