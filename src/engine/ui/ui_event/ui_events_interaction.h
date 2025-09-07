#ifndef MITE_UI_INTERACTION_EVENTS_H
#define MITE_UI_INTERACTION_EVENTS_H

#include "ui_core/ui_style.h"
#include "ui_event.h"
#include <glm/glm.hpp>

namespace mite {
/**
 * @brief 按钮点击事件
 */
class ButtonClickEvent : public UIEvent {
 public:
  explicit ButtonClickEvent(UUID widgetId, const std::string &label = "")
      : m_WidgetId(widgetId), m_Label(label)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetLabel() const
  {
    return m_Label;
  }

  std::string ToString() const override
  {
    return "ButtonClickEvent: " + m_Label + " (ID: " + UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new ButtonClickEvent(m_WidgetId, m_Label);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  std::string m_Label;
};

/**
 * @brief 滑块值改变事件
 */
class SliderChangeEvent : public UIEvent {
 public:
  explicit SliderChangeEvent(UUID widgetId, float value, const std::string &label = "")
      : m_WidgetId(widgetId), m_Value(value), m_Label(label)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  float GetValue() const
  {
    return m_Value;
  }
  const std::string &GetLabel() const
  {
    return m_Label;
  }

  std::string ToString() const override
  {
    return "SliderChangeEvent: " + m_Label + " = " + std::to_string(m_Value) +
           " (ID: " + UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new SliderChangeEvent(m_WidgetId, m_Value, m_Label);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  float m_Value;
  std::string m_Label;
};

/**
 * @brief 复选框切换事件
 */
class CheckboxToggleEvent : public UIEvent {
 public:
  explicit CheckboxToggleEvent(UUID widgetId, bool checked, const std::string &label = "")
      : m_WidgetId(widgetId), m_Checked(checked), m_Label(label)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  bool IsChecked() const
  {
    return m_Checked;
  }
  const std::string &GetLabel() const
  {
    return m_Label;
  }

  std::string ToString() const override
  {
    return "CheckboxToggleEvent: " + m_Label + " = " + (m_Checked ? "checked" : "unchecked") +
           " (ID: " + UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new CheckboxToggleEvent(m_WidgetId, m_Checked, m_Label);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  bool m_Checked;
  std::string m_Label;
};

/**
 * @brief 文本输入事件
 */
class TextInputEvent : public UIEvent {
 public:
  explicit TextInputEvent(UUID widgetId,
                          const std::string &text,
                          const std::string &label = "")
      : m_WidgetId(widgetId), m_Text(text), m_Label(label)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetText() const
  {
    return m_Text;
  }
  const std::string &GetLabel() const
  {
    return m_Label;
  }

  std::string ToString() const override
  {
    return "TextInputEvent: " + m_Label + " = \"" + m_Text + "\"" +
           " (ID: " + UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new TextInputEvent(m_WidgetId, m_Text, m_Label);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  std::string m_Text;
  std::string m_Label;
};

/**
 * @brief 下拉框选择事件
 */
class ComboBoxSelectEvent : public UIEvent {
 public:
  explicit ComboBoxSelectEvent(UUID widgetId,
                               int selectedIndex,
                               const std::string &selectedItem,
                               const std::string &label = "")
      : m_WidgetId(widgetId),
        m_SelectedIndex(selectedIndex),
        m_SelectedItem(selectedItem),
        m_Label(label)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  int GetSelectedIndex() const
  {
    return m_SelectedIndex;
  }
  const std::string &GetSelectedItem() const
  {
    return m_SelectedItem;
  }
  const std::string &GetLabel() const
  {
    return m_Label;
  }

  std::string ToString() const override
  {
    return "ComboBoxSelectEvent: " + m_Label + " = " + m_SelectedItem +
           " (Index: " + std::to_string(m_SelectedIndex) + ", ID: " + UUIDToString(m_WidgetId) +
           ")";
  }

  Event *Clone() const override
  {
    return new ComboBoxSelectEvent(m_WidgetId, m_SelectedIndex, m_SelectedItem, m_Label);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  int m_SelectedIndex;
  std::string m_SelectedItem;
  std::string m_Label;
};

/**
 * @brief 鼠标进入控件区域事件
 */
class MouseEnterEvent : public UIEvent {
 public:
  explicit MouseEnterEvent(UUID widgetId, const glm::vec2 &position)
      : m_WidgetId(widgetId), m_Position(position)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  glm::vec2 GetPosition() const
  {
    return m_Position;
  }

  std::string ToString() const override
  {
    return "MouseEnterEvent: ID " + UUIDToString(m_WidgetId) + " at (" +
           std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ")";
  }

  Event *Clone() const override
  {
    return new MouseEnterEvent(m_WidgetId, m_Position);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
  glm::vec2 m_Position;
};

/**
 * @brief 鼠标离开控件区域事件
 */
class MouseLeaveEvent : public UIEvent {
 public:
  explicit MouseLeaveEvent(UUID widgetId) : m_WidgetId(widgetId) {}

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  std::string ToString() const override
  {
    return "MouseLeaveEvent: ID " + UUIDToString(m_WidgetId);
  }

  Event *Clone() const override
  {
    return new MouseLeaveEvent(m_WidgetId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  UUID m_WidgetId;
};


}  // namespace mite

#endif  // MITE_UI_INTERACTION_EVENTS_H
