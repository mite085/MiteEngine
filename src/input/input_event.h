#ifndef MITE_INPUT_EVENTS
#define MITE_INPUT_EVENTS

#include "headers/headers.h"

namespace mite {

class MouseMoveEvent : public Event {
 public:
  MouseMoveEvent(double xpos, double ypos) : xpos(xpos), ypos(ypos) {}
  glm::vec2 GetPosition() const;
  double GetXPos() const;
  double GetYPos() const;

  EVENT_CLASS_TYPE(MOUSE_POSITION_MOVED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE)
  Event *Clone() const override
  {
    return new MouseMoveEvent(xpos, ypos);
  }
 private:
  double xpos, ypos;
};

class MouseButtonEvent : public Event {
 public:
  MouseButtonEvent(int button, int action, int mods, double xpos, double ypos)
      : button(button), action(action), mods(mods), xpos(xpos), ypos(ypos)
  {
  }
  int GetButton() const;
  int GetAction() const;
  int GetMods() const;
  double GetXPos() const;
  double GetYPos() const;

  EVENT_CLASS_TYPE(MOUSE_BUTTON_RELEASED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE)
  Event *Clone() const override
  {
    return new MouseButtonEvent(button, action, mods, xpos, ypos);
  }
 private:
  int button, action, mods;
  double xpos, ypos;
};

class KeyEvent : public Event {
 public:
  KeyEvent(int key, int scancode, int action, int mods)
      : key(key), scancode(scancode), action(action), mods(mods)
  {
  }
  int GetKey() const;
  int GetScancode() const;
  int GetAction() const;
  int GetMods() const;

  EVENT_CLASS_TYPE(KEY_RELEASED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE)
  Event *Clone() const override
  {
    return new KeyEvent(key, scancode, action, mods);
  }
 private:
  int key, scancode, action /*GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT*/, mods /*ÐÞÊÎ¼ü×´Ì¬*/;
};
};

#endif
