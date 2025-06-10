#ifndef MITE_WINDOW_EVENT
#define MITE_WINDOW_EVENT

#include "headers/headers.h"

namespace mite {

class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)

 private:
  uint32_t m_Width, m_Height;
};

class WindowFocusEvent : public Event {
 public:
  WindowFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowLostFocusEvent : public Event {
 public:
  WindowLostFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowLostFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowMovedEvent : public Event {
 public:
  WindowMovedEvent() = default;
  EVENT_CLASS_TYPE(WindowMoved)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};
};

#endif
