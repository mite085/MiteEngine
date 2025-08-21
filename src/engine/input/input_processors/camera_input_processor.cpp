#include "camera_input_processor.h"
#include "GLFW/glfw3.h"
#include "input/input_manager.h"

namespace mite {

// 日志系统
Logger CameraInputProcessor::s_Logger = nullptr;

CameraInputProcessor::CameraInputProcessor(std::shared_ptr<Camera> camera)
    : m_Camera(std::move(camera))
{
  // 首次创建时初始化日志系统
  if (!s_Logger) {
    s_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Input Processor: Camera");
    s_Logger->trace("Created Input Processor: Camera");
  }
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
bool CameraInputProcessor::handleMouseMove(MouseMoveEvent &e)
{
  if (!m_InputState.rotating && !m_InputState.panning)
    return false;  // 未处理事件

  const glm::vec2 currentPos = {e.GetXPos(), e.GetYPos()};
  const glm::vec2 delta = currentPos - m_LastMousePos;
  m_LastMousePos = currentPos;

  if (m_InputState.rotating) {
    // 右键旋转视角
    m_Camera->Rotate(-delta.x * m_RotationSpeed, delta.y * m_RotationSpeed);
    return true;  // 事件已处理
  }
  else if (m_InputState.panning) {
    // 中键平移视角
    m_Camera->Pan(-delta.x * float(0.01) * m_MoveSpeed, delta.y * float(0.01) * m_MoveSpeed);
    return true;  // 事件已处理
  }

  return false;  // 默认返回未处理
}

bool CameraInputProcessor::handleMouseButtonPressed(MouseButtonPressedEvent &e)
{
  const bool pressed = true;

  if (e.GetButton() == GLFW_MOUSE_BUTTON_RIGHT) {
    // 右键旋转控制
    m_InputState.rotating = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;  // 事件已处理
  }
  else if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    // 中键平移控制
    m_InputState.panning = pressed;
    if (pressed) {
      m_LastMousePos = {e.GetXPos(), e.GetYPos()};
    }
    return true;  // 事件已处理
  }

  return false;  // 未处理其他按钮事件
}

bool CameraInputProcessor::handleMouseButtonReleased(MouseButtonReleasedEvent &e)
{
  const bool pressed = false;

  if (e.GetButton() == GLFW_MOUSE_BUTTON_RIGHT) {
    // 右键旋转控制
    m_InputState.rotating = pressed;
    return true;  // 事件已处理
  }
  else if (e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
    // 中键平移控制
    m_InputState.panning = pressed;
    return true;  // 事件已处理
  }

  return false;  // 未处理其他按钮事件
}

bool CameraInputProcessor::handleMouseScroll(MouseScrollEvent &e)
{
  m_Camera->Zoom(float(e.GetYOffset()) * m_ZoomSpeed);
  return true;  // 滚轮事件始终视为已处理
}

bool CameraInputProcessor::handleKeyPressedEvent(KeyPressedEvent &e)
{
  const bool pressed = true;
  const float value = pressed ? 1.0f : 0.0f;

  // WASD移动控制
  switch (e.GetKey()) {
    case GLFW_KEY_W:
      m_InputState.moveDirection.z = -value;
      return true;  // 事件已处理
    case GLFW_KEY_S:
      m_InputState.moveDirection.z = value;
      return true;
    case GLFW_KEY_A:
      m_InputState.moveDirection.x = -value;
      return true;
    case GLFW_KEY_D:
      m_InputState.moveDirection.x = value;
      return true;
    case GLFW_KEY_Q:
      m_InputState.moveDirection.y = -value;
      return true;
    case GLFW_KEY_E:
      m_InputState.moveDirection.y = value;
      return true;
    default:
      s_Logger->error("Invalid camera input key-code: {}", e.GetKey());
      return false;  // 未处理的按键
  }
}

bool CameraInputProcessor::handleKeyReleasedEvent(KeyReleasedEvent &e)
{
  const bool pressed = false;
  const float value = pressed ? 1.0f : 0.0f;

  // WASD移动控制
  switch (e.GetKey()) {
    case GLFW_KEY_W:
      m_InputState.moveDirection.z = -value;
      return true;  // 事件已处理
    case GLFW_KEY_S:
      m_InputState.moveDirection.z = value;
      return true;
    case GLFW_KEY_A:
      m_InputState.moveDirection.x = -value;
      return true;
    case GLFW_KEY_D:
      m_InputState.moveDirection.x = value;
      return true;
    case GLFW_KEY_Q:
      m_InputState.moveDirection.y = -value;
      return true;
    case GLFW_KEY_E:
      m_InputState.moveDirection.y = value;
      return true;
    default:
      s_Logger->error("Invalid camera input key-code: {}", e.GetKey());
      return false;  // 未处理的按键
  }
}

}  // namespace mite