#ifndef MITE_WINDOW_EVENT
#define MITE_WINDOW_EVENT

#include "headers/headers.h"
#include "subscription_group.h"

namespace mite {

class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
  Event *Clone() const override
  {
    return new WindowCloseEvent();
  }
};

class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
  Event *Clone() const override
  {
    return new WindowResizeEvent(m_Width, m_Height);
  }
 private:
  uint32_t m_Width, m_Height;
};

class WindowFocusEvent : public Event {
 public:
  WindowFocusEvent() = default;
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
  Event *Clone() const override
  {
    return new WindowFocusEvent();
  }
};

class WindowLostFocusEvent : public Event {
 public:
  WindowLostFocusEvent() = default;
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
  Event *Clone() const override
  {
    return new WindowLostFocusEvent();
  }
};

class WindowMovedEvent : public Event {
 public:
  WindowMovedEvent(int xpos, int ypos) : xpos(xpos), ypos(ypos) {}
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_WINDOW)
  Event *Clone() const override
  {
    return new WindowMovedEvent(xpos, ypos);
  }

 private:
  int xpos, ypos;
};

};

#endif
