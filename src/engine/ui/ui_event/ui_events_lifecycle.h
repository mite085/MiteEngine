#ifndef MITE_UI_LIFECYCLE_EVENTS_H
#define MITE_UI_LIFECYCLE_EVENTS_H

#include "ui_event.h"

namespace mite {
/**
 * @brief UI初始化完成事件
 */
class UIInitializedEvent : public UIEvent {
 public:
  UIInitializedEvent() = default;

  std::string ToString() const override
  {
    return "UIInitializedEvent";
  }

  Event *Clone() const override
  {
    return new UIInitializedEvent();
  }
  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief UI关闭事件
 */
class UIShutdownEvent : public UIEvent {
 public:
  UIShutdownEvent() = default;

  std::string ToString() const override
  {
    return "UIShutdownEvent";
  }

  Event *Clone() const override
  {
    return new UIShutdownEvent();
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)
};

/**
 * @brief 控件创建事件
 */
class WidgetCreatedEvent : public UIEvent {
 public:
  explicit WidgetCreatedEvent(uint64_t widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "WidgetCreatedEvent: " + m_WidgetType + " (ID: " + std::to_string(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new WidgetCreatedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 控件销毁事件
 */
class WidgetDestroyedEvent : public UIEvent {
 public:
  explicit WidgetDestroyedEvent(uint64_t widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "WidgetDestroyedEvent: " + m_WidgetType + " (ID: " + std::to_string(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new WidgetDestroyedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 面板打开事件
 */
class PanelOpenedEvent : public UIEvent {
 public:
  explicit PanelOpenedEvent(uint64_t panelId, const std::string &panelName)
      : m_PanelId(panelId), m_PanelName(panelName)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_PanelId;
  }
  const std::string &GetPanelName() const
  {
    return m_PanelName;
  }

  std::string ToString() const override
  {
    return "PanelOpenedEvent: " + m_PanelName + " (ID: " + std::to_string(m_PanelId) + ")";
  }

  Event *Clone() const override
  {
    return new PanelOpenedEvent(m_PanelId, m_PanelName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_PanelId;
  std::string m_PanelName;
};

/**
 * @brief 面板关闭事件
 */
class PanelClosedEvent : public UIEvent {
 public:
  explicit PanelClosedEvent(uint64_t panelId, const std::string &panelName)
      : m_PanelId(panelId), m_PanelName(panelName)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_PanelId;
  }
  const std::string &GetPanelName() const
  {
    return m_PanelName;
  }

  std::string ToString() const override
  {
    return "PanelClosedEvent: " + m_PanelName + " (ID: " + std::to_string(m_PanelId) + ")";
  }

  Event *Clone() const override
  {
    return new PanelClosedEvent(m_PanelId, m_PanelName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_PanelId;
  std::string m_PanelName;
};

/**
 * @brief 布局变化事件
 */
class LayoutChangedEvent : public UIEvent {
 public:
  explicit LayoutChangedEvent(uint64_t layoutId,
                              const std::string &layoutType,
                              const glm::vec2 &newSize)
      : m_LayoutId(layoutId), m_LayoutType(layoutType), m_NewSize(newSize)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_LayoutId;
  }
  const std::string &GetLayoutType() const
  {
    return m_LayoutType;
  }
  glm::vec2 GetNewSize() const
  {
    return m_NewSize;
  }

  std::string ToString() const override
  {
    return "LayoutChangedEvent: " + m_LayoutType + " Size: " + std::to_string(m_NewSize.x) + "x" +
           std::to_string(m_NewSize.y) + " (ID: " + std::to_string(m_LayoutId) + ")";
  }

  Event *Clone() const override
  {
    return new LayoutChangedEvent(m_LayoutId, m_LayoutType, m_NewSize);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_LayoutId;
  std::string m_LayoutType;
  glm::vec2 m_NewSize;
};

/**
 * @brief 焦点获得事件
 */
class FocusGainedEvent : public UIEvent {
 public:
  explicit FocusGainedEvent(uint64_t widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "FocusGainedEvent: " + m_WidgetType + " (ID: " + std::to_string(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new FocusGainedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 焦点失去事件
 */
class FocusLostEvent : public UIEvent {
 public:
  explicit FocusLostEvent(uint64_t widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "FocusLostEvent: " + m_WidgetType + " (ID: " + std::to_string(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new FocusLostEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief UI样式改变事件
 */
class UIStyleChangedEvent : public UIEvent {
 public:
  explicit UIStyleChangedEvent(const std::string &styleName, const std::string &themeName)
      : m_StyleName(styleName), m_ThemeName(themeName)
  {
  }

  const std::string &GetStyleName() const
  {
    return m_StyleName;
  }
  const std::string &GetThemeName() const
  {
    return m_ThemeName;
  }

  std::string ToString() const override
  {
    return "UIStyleChangedEvent: " + m_StyleName + " (Theme: " + m_ThemeName + ")";
  }

  Event *Clone() const override
  {
    return new UIStyleChangedEvent(m_StyleName, m_ThemeName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  std::string m_StyleName;
  std::string m_ThemeName;
};

/**
 * @brief 本地化语言改变事件
 */
class LocalizationChangedEvent : public UIEvent {
 public:
  explicit LocalizationChangedEvent(const std::string &languageCode,
                                    const std::string &languageName)
      : m_LanguageCode(languageCode), m_LanguageName(languageName)
  {
  }

  const std::string &GetLanguageCode() const
  {
    return m_LanguageCode;
  }
  const std::string &GetLanguageName() const
  {
    return m_LanguageName;
  }

  std::string ToString() const override
  {
    return "LocalizationChangedEvent: " + m_LanguageName + " (" + m_LanguageCode + ")";
  }

  Event *Clone() const override
  {
    return new LocalizationChangedEvent(m_LanguageCode, m_LanguageName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  std::string m_LanguageCode;
  std::string m_LanguageName;
};

/**
 * @brief UI可见性改变事件
 */
class UIVisibilityChangedEvent : public UIEvent {
 public:
  explicit UIVisibilityChangedEvent(uint64_t widgetId, const std::string &widgetType, bool visible)
      : m_WidgetId(widgetId), m_WidgetType(widgetType), m_Visible(visible)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }
  bool IsVisible() const
  {
    return m_Visible;
  }

  std::string ToString() const override
  {
    return "UIVisibilityChangedEvent: " + m_WidgetType + (m_Visible ? " SHOWN" : " HIDDEN") +
           " (ID: " + std::to_string(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new UIVisibilityChangedEvent(m_WidgetId, m_WidgetType, m_Visible);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  uint64_t m_WidgetId;
  std::string m_WidgetType;
  bool m_Visible;
};
}  // namespace mite

#endif  // MITE_UI_LIFECYCLE_EVENTS_H
