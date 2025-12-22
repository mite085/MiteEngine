#include "input_state_tracker.h"

namespace mite {
InputStateTracker::InputStateTracker() {
  // 构造函数中重置Timer确保初始状态正确
  m_Timer.Reset();
}

void InputStateTracker::OnKeyPressed(int key) {
  CheckAndResetTimer();

  // 记录按键按下时间戳
  m_KeyStates[key] = m_Timer.ElapsedSeconds();
  m_HasAnyState = true;
}

void InputStateTracker::OnKeyReleased(int key) {
  CheckAndResetTimer();

  // 移除按键状态
  m_KeyStates.erase(key);
  m_HasAnyState = !m_KeyStates.empty() || !m_MouseButtonStates.empty();
}

void InputStateTracker::OnMouseButtonPressed(int button) {
  CheckAndResetTimer();

  // 记录鼠标按键按下时间戳
  m_MouseButtonStates[button] = m_Timer.ElapsedSeconds();
  m_HasAnyState = true;
}

void InputStateTracker::OnMouseButtonReleased(int button) {
  CheckAndResetTimer();

  // 移除鼠标按键状态
  m_MouseButtonStates.erase(button);
  m_HasAnyState = !m_KeyStates.empty() || !m_MouseButtonStates.empty();
}

bool InputStateTracker::IsKeyPressed(int key) const {
  return m_KeyStates.find(key) != m_KeyStates.end();
}

bool InputStateTracker::IsMouseButtonPressed(int button) const {
  return m_MouseButtonStates.find(button) != m_MouseButtonStates.end();
}

float InputStateTracker::GetKeyPressTime(int key) const {
  auto it = m_KeyStates.find(key);
  if (it != m_KeyStates.end()) {
    return it->second;
  }
  return -1.0f;  // 返回-1表示按键未按下
}

float InputStateTracker::GetMouseButtonPressTime(int button) const {
  auto it = m_MouseButtonStates.find(button);
  if (it != m_MouseButtonStates.end()) {
    return it->second;
  }
  return -1.0f;  // 返回-1表示按键未按下
}

std::unordered_set<int> InputStateTracker::GetPressedKeys() const {
  std::unordered_set<int> pressedKeys;
  for (const auto &pair : m_KeyStates) {
    pressedKeys.insert(pair.first);
  }
  return pressedKeys;
}

std::unordered_set<int> InputStateTracker::GetPressedMouseButtons() const {
  std::unordered_set<int> pressedButtons;
  for (const auto &pair : m_MouseButtonStates) {
    pressedButtons.insert(pair.first);
  }
  return pressedButtons;
}

bool InputStateTracker::HasActiveInput() const { return m_HasAnyState; }

void InputStateTracker::ClearAllStates() {
  m_KeyStates.clear();
  m_MouseButtonStates.clear();
  m_HasAnyState = false;
  m_Timer.Reset();  // 重置Timer
}

void InputStateTracker::CheckAndResetTimer() {
  // Timer自洁行为：当有新的输入事件且此前无任何状态记录时，重置Timer
  if (!m_HasAnyState) {
    m_Timer.Reset();
  }
}
}  // namespace mite