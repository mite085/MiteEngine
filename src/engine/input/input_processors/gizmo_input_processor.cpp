#include "gizmo_input_processor.h"
#include "GLFW/glfw3.h"

namespace mite {

GizmoInputProcessor::GizmoInputProcessor(std::shared_ptr<Camera> camera,
                                         glm::mat4 &transformMatrix)
    : m_Camera(std::move(camera)), m_TransformMatrix(transformMatrix)
{
  // 订阅事件
  m_EventSubscriptions.Subscribe<MouseButtonPressedEvent>(BIND_DISPATCH_FN(handleMouseButton));
  m_EventSubscriptions.Subscribe<KeyPressedEvent>(BIND_DISPATCH_FN(handleKey));
}

void GizmoInputProcessor::Update(float deltaTime)
{
  m_Gizmo.Manipulate(m_TransformMatrix, *m_Camera, m_ViewportPos, m_ViewportSize);
}

void GizmoInputProcessor::SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size)
{
  m_ViewportPos = pos;
  m_ViewportSize = size;
}

void GizmoInputProcessor::handleMouseButton(MouseButtonPressedEvent &e)
{
  // 只处理左键点击且Gizmo未被使用时
  if (e.GetButton() == GLFW_MOUSE_BUTTON_LEFT && !m_Gizmo.IsOver()) {
    // TODO: 实现选择逻辑
  }
  return;
}

void GizmoInputProcessor::handleKey(KeyPressedEvent &e)
{
  const bool pressed = (e.GetEventType() == EventType::KEY_PRESSED);
  if (!pressed)
    return ;

  // Gizmo操作切换
  switch (e.GetKey()) {
    case GLFW_KEY_W:  // 平移
      m_Gizmo.SetOperation(ImGuizmo::OPERATION::TRANSLATE);
      break;
    case GLFW_KEY_E:  // 旋转
      m_Gizmo.SetOperation(ImGuizmo::OPERATION::ROTATE);
      break;
    case GLFW_KEY_R:  // 缩放
      m_Gizmo.SetOperation(ImGuizmo::OPERATION::SCALE);
      break;
    case GLFW_KEY_T:  // 切换本地/世界空间
      m_Gizmo.SetMode(m_Gizmo.GetMode() == ImGuizmo::MODE::LOCAL ? ImGuizmo::MODE::WORLD :
                                                                   ImGuizmo::MODE::LOCAL);
      break;
  }
  return;
}

}  // namespace mite