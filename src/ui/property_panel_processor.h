#ifndef MITE_PROPERTY_PANEL_PROCESSOR
#define MITE_PROPERTY_PANEL_PROCESSOR

#include "input.h"
#include "input_processor.h"

namespace mite {

// TODO: Ϊ����ʱ����ɹ�����дһ�������PropertyPanel
class ScreenRect {
 public:
  bool Contains(glm::vec2)
  {
    return true;
  }
  float Left, Top;
};
class PropertyPanel {
 public:
  void OnDrag(glm::vec2, glm::vec2) {}
  bool IsFocused()
  {
    return true;
  }
  bool TestSliderHit(glm::vec2)
  {
    return true;
  }
  ScreenRect GetScreenRect()
  {
    return ScreenRect();
  }
};

class PropertyPanelProcessor : public InputProcessor {
 public:
  explicit PropertyPanelProcessor(PropertyPanel *panel);

  // IInputProcessor�ӿ�ʵ��
  bool HandleEvent(Event &e) override;
  int GetPriority() const override
  {
    return InputPriority::UI_FORM;
  }
  const std::string &GetID() const override
  {
    return m_ID;
  }

  // ���ýӿ�
  void SetActive(bool active)
  {
    m_Active = active;
  }
  void SetHotkeysEnabled(bool enabled)
  {
    m_HotkeysEnabled = enabled;
  }

 private:
  // �¼�������
  bool _HandleMouseClick(MouseButtonEvent &e);
  bool _HandleMouseDrag(MouseMoveEvent &e);
  bool _HandleKeyPress(KeyEvent &e);
  // TODO: ���MouseScrolledEvent�¼���ȫ
  //bool _HandleMouseScroll(MouseScrolledEvent &e);

  // ��������
  bool _IsMouseInPanel() const;
  bool _IsFocused() const;
  glm::vec2 _ToPanelSpace(const glm::vec2 &screenPos) const;

  PropertyPanel *m_Panel;  // �����������
  const std::string m_ID = "PropertyPanel";

  // ״̬����
  bool m_Active = true;
  bool m_HotkeysEnabled = true;
  bool m_IsDragging = false;
  glm::vec2 m_LastMousePos = glm::vec2(0.0, 0.0);

  // ���ò���
  float m_DragThreshold = 5.0f;  // �����϶���ֵ
};
};

#endif
