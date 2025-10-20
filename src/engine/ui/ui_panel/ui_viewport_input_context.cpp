#include "ui_viewport_input_context.h"
#include "basic_event/render_event.h"

namespace mite {
ViewportInputContext::ViewportInputContext(const std::string &name) : InputContext(name)
{
  m_InputStateTracker = std::make_unique<InputStateTracker>();
}

void ViewportInputContext::Update(float deltatime, bool gizmoUsing)
{
  // 更新Gizmo占用
  m_ViewportGizmoUsing = gizmoUsing;

  // 若不处于输入状态，清空状态机，停止更新操作
  if (!m_ViewportFocused || !m_ViewportHovered || m_ViewportGizmoUsing) {
    m_InputStateTracker->ClearAllStates();
    return;
  }

  // 更新相机移动
  UpdateCameraMove(deltatime);
}

void ViewportInputContext::Apply(Transform cameraTransform)
{
  // 更新相机世界空间移动
  glm::vec3 cameraWorldMove = m_CameraMoveCache.x * cameraTransform.GetConstrainedRight() +
                              m_CameraMoveCache.y * cameraTransform.GetConstrainedUp() +
                              m_CameraMoveCache.z * cameraTransform.GetConstrainedForward();
  cameraTransform.Translate(cameraWorldMove);

  // 更新相机旋转
  cameraTransform.RotateCamera(m_CameraRotateCache.x, m_CameraRotateCache.y);

  // 更新相机平移
  cameraTransform.PanCamera(m_CameraPanCache.x, m_CameraPanCache.y);

  // 发布相机变换事件
  EventBus::Publish<ViewportCameraUpdateEvent>(
      ViewportCameraUpdateEvent(cameraTransform, m_CameraZoomCache));

  // 清空缓存
  ClearCameraCache();
}

void ViewportInputContext::SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size)
{
  m_ViewportPos = pos;
  m_ViewportSize = size;
}

void ViewportInputContext::ProcessEvent(Event &e)
{
  // 检查视口是否聚焦，且鼠标悬停，Gizmo非占用，处于可输入状态
  if (!m_ViewportFocused || !m_ViewportHovered || m_ViewportGizmoUsing) {
    // 若不处于输入状态，清空状态机
    m_InputStateTracker->ClearAllStates();
    return;
  }

  // 执行原有逻辑，分发事件
  InputContext::ProcessEvent(e);
}

void ViewportInputContext::ProcessMouseMoveEvent(MouseMoveEvent &e)
{
  // 记录此次更新
  const glm::vec2 currentPos = {e.GetXPos(), e.GetYPos()};
  const glm::vec2 delta = currentPos - m_LastMousePos;
  m_LastMousePos = currentPos;

  // 右键旋转操作累积（此处为减号，确保方向正确）
  if (m_InputStateTracker->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    m_CameraRotateCache -= delta * m_CameraRotationSpeed;
  }

  // 中键平移操作累积（此处限制速度）
  if (m_InputStateTracker->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
    m_CameraPanCache += delta * m_CameraMoveSpeed * 0.01f;
  }
}

void ViewportInputContext::ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e)
{
  // 更新状态机
  m_InputStateTracker->OnMouseButtonPressed(e.GetButton());
}

void ViewportInputContext::ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e)
{
  // 更新状态机
  m_InputStateTracker->OnMouseButtonReleased(e.GetButton());

  // 左键释放时，执行选择
  if (e.GetButton() == GLFW_MOUSE_BUTTON_LEFT) {
    glm::vec2 uv = ScreenToUV({e.GetXPos(), e.GetYPos()});
    EventBus::Publish<ViewportPickedEvent>(ViewportPickedEvent(uv));
  }
}

void ViewportInputContext::ProcessMouseScrollEvent(MouseScrollEvent &e)
{
  // 滚轮缩放操作累积
  // 常规鼠标仅存在Y方向鼠标，仅需处理Y轴。
  m_CameraZoomCache += float(e.GetYOffset()) * m_CameraZoomSpeed;
}

void ViewportInputContext::ProcessKeyPressdEvent(KeyPressedEvent &e)
{
  // 更新状态机
  m_InputStateTracker->OnKeyPressed(e.GetKey());
}

void ViewportInputContext::ProcessKeyReleasedEvent(KeyReleasedEvent &e)
{
  // 更新状态机
  m_InputStateTracker->OnKeyReleased(e.GetKey());
}

void ViewportInputContext::ProcessKeyTypedEvent(KeyTypedEvent &e) {}

glm::vec2 ViewportInputContext::ScreenToUV(const glm::vec2 &screenPos)
{
  // 1. 计算相对于视口左上角的局部坐标
  glm::vec2 localPos = screenPos - m_ViewportPos;

  // 2. 转换为UV坐标（X轴正常，Y轴翻转）
  glm::vec2 uv;
  uv.x = localPos.x / m_ViewportSize.x;           // X: 0~1 从左到右
  uv.y = 1.0f - (localPos.y / m_ViewportSize.y);  // Y: 0~1 从下到上（翻转Y轴）

  // 3. 钳制到[0,1]范围，防止越界
  uv.x = glm::clamp(uv.x, 0.0f, 1.0f);
  uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

  return uv;
}
glm::vec2 ViewportInputContext::UVToScreen(const glm::vec2 &uv)
{
  // 钳制UV坐标到有效范围
  glm::vec2 clampedUV = glm::clamp(uv, 0.0f, 1.0f);

  // 转换为屏幕坐标
  glm::vec2 screenPos;
  screenPos.x = m_ViewportPos.x + clampedUV.x * m_ViewportSize.x;
  screenPos.y = m_ViewportPos.y + (1.0f - clampedUV.y) * m_ViewportSize.y;  // 翻转Y轴

  return screenPos;
}

void ViewportInputContext::UpdateCameraMove(float deltatime)
{
  // 相机移动方向
  glm::vec3 cameraMoveDirection = glm::vec3(0.0f);

  // 根据所有按键状态计算最终移动方向（避免按键竞争，当WS同时按下时，互相抵消）
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_W))
    // 与GetForwardVector()相乘，W按键(前进)为正
    cameraMoveDirection.z += 1.0f;
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_S))
    cameraMoveDirection.z -= 1.0f;
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_A))
    cameraMoveDirection.x -= 1.0f;
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_D))
    // 与GetRightVector()相乘，R按键(向右)为正
    cameraMoveDirection.x += 1.0f;
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_Q))
    cameraMoveDirection.y -= 1.0f;
  if (m_InputStateTracker->IsKeyPressed(GLFW_KEY_E))
    // 与GetUpVector()相乘，E按键(向上)为正
    cameraMoveDirection.y += 1.0f;

  // 归一化
  if (glm::length(cameraMoveDirection) > 1.0f) {
    cameraMoveDirection = glm::normalize(cameraMoveDirection);
  }

  // 计算相机移动缓存
  m_CameraMoveCache = cameraMoveDirection * m_CameraMoveSpeed * deltatime;
}


void ViewportInputContext::ClearCameraCache()
{
  m_CameraRotateCache = {0.0f, 0.0f};
  m_CameraPanCache = {0.0f, 0.0f};
  m_CameraZoomCache = 0.0f;
}
}  // namespace mite