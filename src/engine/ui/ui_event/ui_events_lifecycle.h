#ifndef MITE_UI_LIFECYCLE_EVENTS_H
#define MITE_UI_LIFECYCLE_EVENTS_H

#include "ui_event.h"
#include "ui_core/ui_style.h"

namespace mite {
/**
 * @brief UI初始化完成事件
 */
class UISystemInitializedEvent : public Event {
 public:
  UISystemInitializedEvent() = default;

  std::string ToString() const override
  {
    return "UIInitializedEvent";
  }

  Event *Clone() const override
  {
    return new UISystemInitializedEvent();
  }
  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief UI关闭事件
 */
class UISystemShutdownEvent : public Event {
 public:
  UISystemShutdownEvent() = default;

  std::string ToString() const override
  {
    return "UIShutdownEvent";
  }

  Event *Clone() const override
  {
    return new UISystemShutdownEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief 面板打开事件
 */
class PanelOpenedEvent : public UIEvent {
 public:
  explicit PanelOpenedEvent(std::shared_ptr<UIPanel> panel)
      : UIEvent(panel)
  {
  }

  Event *Clone() const override
  {
    return new PanelOpenedEvent(m_Panel);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief 面板关闭事件
 */
class PanelClosedEvent : public UIEvent {
 public:
  explicit PanelClosedEvent(std::shared_ptr<UIPanel> panel)
      : UIEvent(panel)
  {
  }
  Event *Clone() const override { return new PanelClosedEvent(m_Panel); }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief 样式变更事件
 * 当样式或样式属性发生变化时触发
 */
struct StyleChangedEvent : public Event {
 public:
  explicit StyleChangedEvent(std::shared_ptr<UIStyle> style)
      : m_Style(style)
  {
  }
  std::shared_ptr<UIStyle> GetUIStyle() const
  {
    return m_Style;
  }
  Event *Clone() const override
  {
    return new StyleChangedEvent(m_Style);
  }
  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  std::shared_ptr<UIStyle> m_Style;  // 样式名称
};

/**
 * @brief 语言变更事件
 */
class LanguageChangedEvent : public Event {
 public:
  explicit LanguageChangedEvent(const std::string &newLanguageCode)
      : m_NewLanguageCode(newLanguageCode)
  {
  }
  const std::string &GetNewLanguage() const
  {
    return m_NewLanguageCode;
  }

  std::string ToString() const override
  {
    return "LanguageChangedEvent: " + m_NewLanguageCode;
  }

  Event *Clone() const override
  {
    return new LanguageChangedEvent(m_NewLanguageCode);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_INTERACTION)

 private:
  std::string m_NewLanguageCode;
};
}  // namespace mite

#endif  // MITE_UI_LIFECYCLE_EVENTS_H
