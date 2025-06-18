#ifndef MITE_WINDOW_EVENT
#define MITE_WINDOW_EVENT

#include "headers/headers.h"

namespace mite {

class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  EVENT_CLASS_TYPE(WINDOW_CLOSE)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
};

class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  EVENT_CLASS_TYPE(WINDOW_RESIZE)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)

 private:
  uint32_t m_Width, m_Height;
};

class WindowFocusEvent : public Event {
 public:
  WindowFocusEvent() = default;
  EVENT_CLASS_TYPE(WINDOW_FOCUS)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
};

class WindowLostFocusEvent : public Event {
 public:
  WindowLostFocusEvent() = default;
  EVENT_CLASS_TYPE(WINDOW_LOST_FOCUS)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
};

class WindowMovedEvent : public Event {
 public:
  WindowMovedEvent() = default;
  EVENT_CLASS_TYPE(WINDOW_MOVED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
};
};

#endif
