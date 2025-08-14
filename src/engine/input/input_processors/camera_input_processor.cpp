#include "camera_input_processor.h"
#include "GLFW/glfw3.h"
#include "input/input_manager.h"

namespace mite {

CameraInputProcessor::CameraInputProcessor(std::shared_ptr<Camera> camera)
    : m_Camera(std::move(camera))
{
  // 订阅事件
  m_EventSubscriptions.Subscribe<MouseMoveEvent>(BIND_DISPATCH_FN(handleMouseMove));
  m_EventSubscriptions.Subscribe<MouseButtonReleasedEvent>(BIND_DISPATCH_FN(handleMouseButton));
  m_EventSubscriptions.Subscribe<MouseScrollEvent>(BIND_DISPATCH_FN(handleMouseScroll));
  m_EventSubscriptions.Subscribe<KeyReleasedEvent>(BIND_DISPATCH_FN(handleKeyEvent));
}

void CameraInputProcessor::UpdateCameraTransform(float deltaTime)
{
  if (glm::length(m_InputState.moveDirection) > 0.01f) {
    // 标准化移动方向并应用速度和帧时间
    glm::vec3 moveDir = glm::normalize(m_InputState.moveDirection);
    glm::vec3 worldMove = moveDir.x * m_Camera->GetRightVector() +
                          moveDir.y * m_Camera->GetUpVector() +
                          moveDir.z * m_Camera->GetForwardVector();

    m_Camera->Move(worldMove * m_MoveSpeed * deltaTime);
  }
}

// --- 私有方法实现 ---
void CameraInputProcessor::handleMouseMove(MouseMoveEvent &e)
{
  if (!m_InputState.rotating && !m_InputState.panning)
    return;

  const glm::vec2 currentPos = {e.GetXPos(), e.GetYPos()};
  const glm::vec2 delta = currentPos - m_LastMousePos;
  m_LastMousePos = currentPos;

  if (m_InputState.rotating) {
    // 右键旋转视角
    m_Camera->Rotate(-delta.x * m_RotationSpeed, delta.y * m_RotationSpeed);
  }
  else if (m_InputState.panning) {
    // 中键平移视角
    m_Camera->Pan(-delta.x * 0.01f * m_MoveSpeed, delta.y * 0.01f * m_MoveSpeed);
  }
}

void CameraInputProcessor::handleMouseButton(MouseButtonReleasedEvent &e)
{
  const bool pressed = (e.GetEventType() == EventType::MOUSE_BUTTON_RELEASED);

  if (e.GetButton() == GLFW_MOUSE_BUTTON_RIGHT) {
    // 右键旋转控制
    m_InputState.rotating = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
  }
  else if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    // 中键平移控制
    m_InputState.panning = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
  }
}

void CameraInputProcessor::handleMouseScroll(MouseScrollEvent &e)
{
  m_Camera->Zoom(float(e.GetYOffset()) * m_ZoomSpeed);
}

void CameraInputProcessor::handleKeyEvent(KeyReleasedEvent &e)
{
  const bool pressed = (e.GetEventType() == EventType::KEY_RELEASED);
  const float value = pressed ? 1.0f : 0.0f;

  // WASD移动控制
  switch (e.GetKey()) {
    case GLFW_KEY_W:
      m_InputState.moveDirection.z = -value;
      break;
    case GLFW_KEY_S:
      m_InputState.moveDirection.z = value;
      break;
    case GLFW_KEY_A:
      m_InputState.moveDirection.x = -value;
      break;
    case GLFW_KEY_D:
      m_InputState.moveDirection.x = value;
      break;
    case GLFW_KEY_Q:
      m_InputState.moveDirection.y = -value;
      break;
    case GLFW_KEY_E:
      m_InputState.moveDirection.y = value;
      break;
    default:
      m_Logger->error("Invalid camera input key-code: {}", e.GetKey());
      return ;
  }
}

}  // namespace mite