#include "input_event.h"

namespace mite {
glm::vec2 MouseMoveEvent::GetPosition() const
{
  return glm::vec2(xpos, ypos);
}
double MouseMoveEvent::GetXPos() const
{
  return xpos;
}
double MouseMoveEvent::GetYPos() const
{
  return ypos;
}
int MouseButtonEvent::GetButton() const
{
  return button;
}
int MouseButtonEvent::GetAction() const
{
  return action;
}
int MouseButtonEvent::GetMods() const
{
  return mods;
}
double MouseButtonEvent::GetXPos() const
{
  return 0.0;
}
double MouseButtonEvent::GetYPos() const
{
  return 0.0;
}
int KeyEvent::GetKey() const
{
  return key;
}
int KeyEvent::GetScancode() const
{
  return scancode;
}
int KeyEvent::GetAction() const
{
  return action;
}
int KeyEvent::GetMods() const
{
  return mods;
}
};
