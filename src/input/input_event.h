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

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
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

  EVENT_CLASS_TYPE(MouseButtonReleased)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
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

  EVENT_CLASS_TYPE(KeyReleased)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
 private:
  int key, scancode, action /*GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT*/, mods /*ÐÞÊÎ¼ü×´Ì¬*/;
};
};

#endif
