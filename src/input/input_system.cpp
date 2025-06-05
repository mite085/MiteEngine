#include "input_system.h"

void mite::InputSystem::OnEvent(Event &event) {}

bool mite::InputSystem::IsKeyPressed(KeyCode key) const
{
  return false;
}

bool mite::InputSystem::IsMouseButtonPressed(MouseButton button) const
{
  return false;
}

glm::vec2 mite::InputSystem::GetMousePosition() const
{
  return glm::vec2();
}

float mite::InputSystem::GetMouseScrollDelta() const
{
  return 0.0f;
}

void mite::InputSystem::AddActionMapping(const std::string &action, KeyCode key) {}

bool mite::InputSystem::GetAction(const std::string &action) const
{
  return false;
}

void mite::InputSystem::Update() {}
