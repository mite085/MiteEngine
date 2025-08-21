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
    m_Camera->Rotate(-delta.x * m_RotationSpeed, -delta.y * m_RotationSpeed);
    return true;  // 事件已处理
  }
  else if (m_InputState.panning) {
    // 中键平移视角
    m_Camera->Pan(delta.x * float(0.01) * m_MoveSpeed, -delta.y * float(0.01) * m_MoveSpeed);
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
  // 常规鼠标仅存在Y方向鼠标，仅需处理Y轴。
  m_Camera->Zoom(float(e.GetYOffset()) * m_ZoomSpeed);
  return true;  // 滚轮事件始终视为已处理
}

bool CameraInputProcessor::handleKeyPressedEvent(KeyPressedEvent &e)
{
  // 记录按键按下状态
  m_InputState.keyStates[e.GetKey()] = true;
  UpdateMoveDirection();
  return true;
}

bool CameraInputProcessor::handleKeyReleasedEvent(KeyReleasedEvent &e)
{
  // 记录按键释放状态
  m_InputState.keyStates[e.GetKey()] = false;
  UpdateMoveDirection();
  return true;
}

void CameraInputProcessor::UpdateMoveDirection()
{
  // 重置移动方向
  m_InputState.moveDirection = glm::vec3(0.0f);

  // 根据所有按键状态计算最终移动方向（避免按键竞争，当WS同时按下时，互相抵消）
  if (m_InputState.keyStates[GLFW_KEY_W])
    // 与GetForwardVector()相乘，W按键(前进)为正
    m_InputState.moveDirection.z += 1.0f;   
  if (m_InputState.keyStates[GLFW_KEY_S])
    m_InputState.moveDirection.z -= 1.0f;
  if (m_InputState.keyStates[GLFW_KEY_A])
    m_InputState.moveDirection.x -= 1.0f;
  if (m_InputState.keyStates[GLFW_KEY_D])
    // 与GetRightVector()相乘，R按键(向右)为正
    m_InputState.moveDirection.x += 1.0f;   
  if (m_InputState.keyStates[GLFW_KEY_Q])
    m_InputState.moveDirection.y -= 1.0f;
  if (m_InputState.keyStates[GLFW_KEY_E])
    // 与GetUpVector()相乘，E按键(向上)为正
    m_InputState.moveDirection.y += 1.0f;   

  // 可选：归一化对角线移动
  if (glm::length(m_InputState.moveDirection) > 1.0f) {
    m_InputState.moveDirection = glm::normalize(m_InputState.moveDirection);
  }
}

}  // namespace mite