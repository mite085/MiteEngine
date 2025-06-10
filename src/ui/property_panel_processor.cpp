#include "property_panel_processor.h"

namespace mite {
PropertyPanelProcessor::PropertyPanelProcessor(PropertyPanel *panel)
    : InputProcessor(), m_Panel(panel)
{
}

bool PropertyPanelProcessor::HandleEvent(Event &e)
{
  if (!m_Active)
    return false;

  EventDispatcher dispatcher(e);

  // ���¼����ͷַ�����
  bool handled = false;
  dispatcher.Dispatch<MouseButtonEvent>(BIND_DISPATCH_FN(_HandleMouseClick));

  dispatcher.Dispatch<MouseMoveEvent>(BIND_DISPATCH_FN(_HandleMouseDrag));

  dispatcher.Dispatch<KeyEvent>(BIND_DISPATCH_FN(_HandleKeyPress));

  //dispatcher.Dispatch<MouseScrolledEvent>(BIND_DISPATCH_FN(_HandleMouseScroll));

  return handled;
}

//--- ��������� ---
bool PropertyPanelProcessor::_HandleMouseClick(MouseButtonEvent &e)
{
  if (!_IsMouseInPanel())
    return false;

  // TODO: ���MouseButton�����ȫ
  if (e.GetButton()/* == MouseButton::Left*/) {
    m_LastMousePos = Input::GetMousePosition();

    // ����Ƿ����˿��϶��ؼ�
    auto panelPos = _ToPanelSpace(m_LastMousePos);
    if (m_Panel->TestSliderHit(panelPos)) {
      m_IsDragging = true;
      return true;
    }
  }
  return false;
}

//--- ����϶����� ---
bool PropertyPanelProcessor::_HandleMouseDrag(MouseMoveEvent &e)
{
  auto currentPos = e.GetPosition();
  auto delta = currentPos - m_LastMousePos;

  // ����Ƿ�ﵽ�϶���ֵ
  if (!m_IsDragging && glm::length(delta) < m_DragThreshold) {
    return false;
  }

  m_IsDragging = true;
  auto panelPos = _ToPanelSpace(currentPos);
  m_Panel->OnDrag(panelPos, delta);
  m_LastMousePos = currentPos;
  return true;
}

//--- ���̿�ݼ����� ---
bool PropertyPanelProcessor::_HandleKeyPress(KeyEvent &e)
{
  if (!m_HotkeysEnabled)
    return false;

  // TODO: ���Key�����ȫ
  //bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);

  //switch (e.GetKeyCode()) {
  //  case Key::C:
  //    if (ctrl)
  //      return m_Panel->CopySelectedProperty();
  //  case Key::V:
  //    if (ctrl)
  //      return m_Panel->PasteToSelectedProperty();
  //  case Key::D:
  //    if (ctrl)
  //      return m_Panel->ResetSelectedProperty();
  //  case Key::Up:
  //    return m_Panel->Navigate(-1);  // ��һ������
  //  case Key::Down:
  //    return m_Panel->Navigate(1);  // ��һ������
  //}
  return false;
}

//--- �����ִ��� ---
//bool PropertyPanelProcessor::_HandleMouseScroll(MouseScrolledEvent &e)
//{
//  if (m_Panel->IsNumericFieldHovered()) {
//    float delta = e.GetDelta() * 0.1f;  // ��ϸ����
//    return m_Panel->AdjustNumberField(delta);
//  }
//  return false;
//}

//--- �������� ---
bool PropertyPanelProcessor::_IsMouseInPanel() const
{
  auto mousePos = Input::GetMousePosition();
  return m_Panel->GetScreenRect().Contains(mousePos);
}

bool PropertyPanelProcessor::_IsFocused() const
{
  return m_Panel->IsFocused() && _IsMouseInPanel();
}

glm::vec2 PropertyPanelProcessor::_ToPanelSpace(const glm::vec2 &screenPos) const
{
  auto rect = m_Panel->GetScreenRect();
  return screenPos - glm::vec2(rect.Left, rect.Top);
}
};
