#ifndef MITE_INPUT_SYSTEM
#define MITE_INPUT_SYSTEM

#include "headers.h"
#include "event/event.h"
#include "glm/glm.hpp"

namespace mite {
enum class KeyCode {
  Space = 32,
  A = 65,
  D = 68,
  Up = 265,
  Down = 264  // GLFW����ֵ
};

enum class MouseButton {
};

class InputSystem {
 public:
  void OnEvent(Event &event);  // ��Window����ԭʼ�¼�

  // ״̬��ѯ
  bool IsKeyPressed(KeyCode key) const;
  bool IsMouseButtonPressed(MouseButton button) const;
  glm::vec2 GetMousePosition() const;
  float GetMouseScrollDelta() const;

  // ����ӳ�䣨��������ӳ�䵽�߼�������
  void AddActionMapping(const std::string &action, KeyCode key);
  bool GetAction(const std::string &action) const;

  // ֡���£����˲ʱ״̬���簴�����£�
  void Update();

 private:
  struct State {
    std::unordered_map<KeyCode, bool> keys;
    std::unordered_map<MouseButton, bool> mouseButtons;
    glm::vec2 mousePosition;
    float scrollDelta = 0.0f;
  };

  State m_CurrentState;
  State m_PreviousState;
  std::unordered_map<std::string, KeyCode> m_ActionMappings;
};
}  // namespace mite

#endif
