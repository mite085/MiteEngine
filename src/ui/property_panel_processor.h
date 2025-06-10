#ifndef MITE_PROPERTY_PANEL_PROCESSOR
#define MITE_PROPERTY_PANEL_PROCESSOR

#include "input.h"
#include "input_processor.h"

namespace mite {

// TODO: 为了暂时编译成功，编写一个假想的PropertyPanel
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

  // IInputProcessor接口实现
  bool HandleEvent(Event &e) override;
  int GetPriority() const override
  {
    return InputPriority::UI_FORM;
  }
  const std::string &GetID() const override
  {
    return m_ID;
  }

  // 配置接口
  void SetActive(bool active)
  {
    m_Active = active;
  }
  void SetHotkeysEnabled(bool enabled)
  {
    m_HotkeysEnabled = enabled;
  }

 private:
  // 事件处理方法
  bool _HandleMouseClick(MouseButtonEvent &e);
  bool _HandleMouseDrag(MouseMoveEvent &e);
  bool _HandleKeyPress(KeyEvent &e);
  // TODO: 添加MouseScrolledEvent事件后补全
  //bool _HandleMouseScroll(MouseScrolledEvent &e);

  // 辅助方法
  bool _IsMouseInPanel() const;
  bool _IsFocused() const;
  glm::vec2 _ToPanelSpace(const glm::vec2 &screenPos) const;

  PropertyPanel *m_Panel;  // 所属属性面板
  const std::string m_ID = "PropertyPanel";

  // 状态跟踪
  bool m_Active = true;
  bool m_HotkeysEnabled = true;
  bool m_IsDragging = false;
  glm::vec2 m_LastMousePos = glm::vec2(0.0, 0.0);

  // 配置参数
  float m_DragThreshold = 5.0f;  // 像素拖动阈值
};
};

#endif
