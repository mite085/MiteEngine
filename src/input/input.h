#ifndef MITE_INPUT
#define MITE_INPUT

#include "event/event.h"
#include "glm/glm.hpp"
#include "headers.h"
#include "input_context.h"

namespace mite {

class Input {
 public:
  static void Init(const std::shared_ptr<InputContextStack> &stack);
  static void Shutdown();
  static void Update();

  // ����״̬��ѯ
  static bool IsKeyPressed(KeyCode key);
  static bool IsKeyReleased(KeyCode key);
  static bool IsKeyHeld(KeyCode key);
  static float GetKeyHeldDuration(KeyCode key);

  // ���״̬��ѯ
  static bool IsMouseButtonPressed(MouseCode button);
  static bool IsMouseButtonReleased(MouseCode button);
  static bool IsMouseButtonHeld(MouseCode button);
  static glm::vec2 GetMousePosition();
  static glm::vec2 GetMouseDelta();
  static float GetMouseScrollDelta(MouseCode button);

  // ���������Ĺ���
  static void PushContext(const std::string &contextName);
  static void PopContext();
  static bool IsCurrentContext(const std::string &contextName);

  // TODO: �¼��ص�
  static void OnEvent(Event &event);
};


}  // namespace mite

#endif
