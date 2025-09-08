#ifndef MITE_UI_LIFECYCLE_EVENTS_H
#define MITE_UI_LIFECYCLE_EVENTS_H

#include "ui_event.h"
#include "ui_core/ui_style.h"

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
  explicit WidgetCreatedEvent(UUID widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "WidgetCreatedEvent: " + m_WidgetType +
           " (ID: " + UUIDGenerator::UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new WidgetCreatedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 控件销毁事件
 */
class WidgetDestroyedEvent : public UIEvent {
 public:
  explicit WidgetDestroyedEvent(UUID widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "WidgetDestroyedEvent: " + m_WidgetType +
           " (ID: " + UUIDGenerator::UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new WidgetDestroyedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 面板打开事件
 */
class PanelOpenedEvent : public UIEvent {
 public:
  explicit PanelOpenedEvent(UUID panelId, const std::string &panelName)
      : m_PanelId(panelId), m_PanelName(panelName)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_PanelId;
  }
  const std::string &GetPanelName() const
  {
    return m_PanelName;
  }

  std::string ToString() const override
  {
    return "PanelOpenedEvent: " + m_PanelName + " (ID: " + UUIDGenerator::UUIDToString(m_PanelId) +
           ")";
  }

  Event *Clone() const override
  {
    return new PanelOpenedEvent(m_PanelId, m_PanelName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_PanelId;
  std::string m_PanelName;
};

/**
 * @brief 面板关闭事件
 */
class PanelClosedEvent : public UIEvent {
 public:
  explicit PanelClosedEvent(UUID panelId, const std::string &panelName)
      : m_PanelId(panelId), m_PanelName(panelName)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_PanelId;
  }
  const std::string &GetPanelName() const
  {
    return m_PanelName;
  }

  std::string ToString() const override
  {
    return "PanelClosedEvent: " + m_PanelName + " (ID: " + UUIDGenerator::UUIDToString(m_PanelId) +
           ")";
  }

  Event *Clone() const override
  {
    return new PanelClosedEvent(m_PanelId, m_PanelName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_PanelId;
  std::string m_PanelName;
};

/**
 * @brief 布局变化事件
 */
class LayoutChangedEvent : public UIEvent {
 public:
  explicit LayoutChangedEvent(UUID layoutId,
                              const std::string &layoutType,
                              const glm::vec2 &newSize)
      : m_LayoutId(layoutId), m_LayoutType(layoutType), m_NewSize(newSize)
  {
  }

  UUID GetSourceWidgetID() const override
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
           std::to_string(m_NewSize.y) + " (ID: " + UUIDGenerator::UUIDToString(m_LayoutId) + ")";
  }

  Event *Clone() const override
  {
    return new LayoutChangedEvent(m_LayoutId, m_LayoutType, m_NewSize);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_LayoutId;
  std::string m_LayoutType;
  glm::vec2 m_NewSize;
};

/**
 * @brief 焦点获得事件
 */
class FocusGainedEvent : public UIEvent {
 public:
  explicit FocusGainedEvent(UUID widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "FocusGainedEvent: " + m_WidgetType +
           " (ID: " + UUIDGenerator::UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new FocusGainedEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 焦点失去事件
 */
class FocusLostEvent : public UIEvent {
 public:
  explicit FocusLostEvent(UUID widgetId, const std::string &widgetType)
      : m_WidgetId(widgetId), m_WidgetType(widgetType)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }
  const std::string &GetWidgetType() const
  {
    return m_WidgetType;
  }

  std::string ToString() const override
  {
    return "FocusLostEvent: " + m_WidgetType + " (ID: " + UUIDGenerator::UUIDToString(m_WidgetId) +
           ")";
  }

  Event *Clone() const override
  {
    return new FocusLostEvent(m_WidgetId, m_WidgetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_WidgetId;
  std::string m_WidgetType;
};

/**
 * @brief 样式变更事件
 * 当样式或样式属性发生变化时触发
 */
struct StyleChangedEvent : public UIEvent {
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
class LanguageChangedEvent : public UIEvent {
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

/**
 * @brief UI可见性改变事件
 */
class UIVisibilityChangedEvent : public UIEvent {
 public:
  explicit UIVisibilityChangedEvent(UUID widgetId, const std::string &widgetType, bool visible)
      : m_WidgetId(widgetId), m_WidgetType(widgetType), m_Visible(visible)
  {
  }

  UUID GetSourceWidgetID() const override
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
           " (ID: " + UUIDGenerator::UUIDToString(m_WidgetId) + ")";
  }

  Event *Clone() const override
  {
    return new UIVisibilityChangedEvent(m_WidgetId, m_WidgetType, m_Visible);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LIFECYCLE)

 private:
  UUID m_WidgetId;
  std::string m_WidgetType;
  bool m_Visible;
};
}  // namespace mite

#endif  // MITE_UI_LIFECYCLE_EVENTS_H
