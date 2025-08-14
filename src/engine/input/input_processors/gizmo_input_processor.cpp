#include "gizmo_input_processor.h"
#include "GLFW/glfw3.h"

namespace mite {
GizmoInputProcessor::GizmoInputProcessor(std::shared_ptr<Camera> camera,
                                         glm::mat4 &transformMatrix)
    : m_Camera(std::move(camera)), m_TransformMatrix(transformMatrix)
{
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

void GizmoInputProcessor::SetTransform(glm::mat4 &transformMatrix)
{
  m_TransformMatrix = transformMatrix;
}

bool GizmoInputProcessor::handleMouseButton(MouseButtonPressedEvent &e)
{
  // 只处理左键点击且Gizmo未被使用时
  if (e.GetButton() == GLFW_MOUSE_BUTTON_LEFT && !m_Gizmo.IsOver()) {
    // TODO: 实现选择逻辑
    return true;  // 事件已处理，阻止传播
  }
  return false;  // 事件未处理，允许其他处理器继续
}

bool GizmoInputProcessor::handleKey(KeyPressedEvent &e)
{
  const bool pressed = (e.GetEventType() == EventType::KEY_PRESSED);
  if (!pressed) {
    return false;  // 忽略按键释放事件
  }

  // Gizmo操作切换
  bool handled = true;  // 默认认为已处理
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
    default:
      handled = false;  // 非Gizmo相关按键，未处理
  }
  return handled;
}
}  // namespace mite