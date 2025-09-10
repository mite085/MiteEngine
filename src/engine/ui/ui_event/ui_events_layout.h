#ifndef MITE_UI_LAYOUT_EVENTS_H
#define MITE_UI_LAYOUT_EVENTS_H

#include "ui_event.h"

namespace mite {

/**
 * @brief 控件位置改变事件
 */
class WidgetPositionChangedEvent : public UIEvent {
 public:
  explicit WidgetPositionChangedEvent(UUID widgetId,
                                      const glm::vec2 &oldPosition,
                                      const glm::vec2 &newPosition)
      : m_WidgetId(widgetId), m_OldPosition(oldPosition), m_NewPosition(newPosition)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  const glm::vec2 &GetOldPosition() const
  {
    return m_OldPosition;
  }
  const glm::vec2 &GetNewPosition() const
  {
    return m_NewPosition;
  }

  std::string ToString() const override
  {
    return "WidgetPositionChangedEvent: ID " + UUIDGenerator::UUIDToString(m_WidgetId) +
           " from (" +
           std::to_string(m_OldPosition.x) + ", " + std::to_string(m_OldPosition.y) + ") to (" +
           std::to_string(m_NewPosition.x) + ", " + std::to_string(m_NewPosition.y) + ")";
  }

  Event *Clone() const override
  {
    return new WidgetPositionChangedEvent(m_WidgetId, m_OldPosition, m_NewPosition);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
  glm::vec2 m_OldPosition;
  glm::vec2 m_NewPosition;
};

/**
 * @brief 控件大小改变事件
 */
class WidgetSizeChangedEvent : public UIEvent {
 public:
  explicit WidgetSizeChangedEvent(UUID widgetId,
                                  const glm::vec2 &oldSize,
                                  const glm::vec2 &newSize)
      : m_WidgetId(widgetId), m_OldSize(oldSize), m_NewSize(newSize)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  const glm::vec2 &GetOldSize() const
  {
    return m_OldSize;
  }
  const glm::vec2 &GetNewSize() const
  {
    return m_NewSize;
  }

  std::string ToString() const override
  {
    return "WidgetSizeChangedEvent: ID " + UUIDGenerator::UUIDToString(m_WidgetId) + " from " +
           std::to_string(m_OldSize.x) + "x" + std::to_string(m_OldSize.y) + " to " +
           std::to_string(m_NewSize.x) + "x" + std::to_string(m_NewSize.y);
  }

  Event *Clone() const override
  {
    return new WidgetSizeChangedEvent(m_WidgetId, m_OldSize, m_NewSize);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
  glm::vec2 m_OldSize;
  glm::vec2 m_NewSize;
};

/**
 * @brief 控件可见性改变事件
 */
class WidgetVisibilityChangedEvent : public UIEvent {
 public:
  explicit WidgetVisibilityChangedEvent(UUID widgetId, bool oldVisible, bool newVisible)
      : m_WidgetId(widgetId), m_OldVisible(oldVisible), m_NewVisible(newVisible)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  bool GetOldVisibility() const
  {
    return m_OldVisible;
  }
  bool GetNewVisibility() const
  {
    return m_NewVisible;
  }

  std::string ToString() const override
  {
    return "WidgetVisibilityChangedEvent: ID " + UUIDGenerator::UUIDToString(m_WidgetId) +
           " from " +
           (m_OldVisible ? "visible" : "hidden") + " to " + (m_NewVisible ? "visible" : "hidden");
  }

  Event *Clone() const override
  {
    return new WidgetVisibilityChangedEvent(m_WidgetId, m_OldVisible, m_NewVisible);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
  bool m_OldVisible;
  bool m_NewVisible;
};

/**
 * @brief 控件启用状态改变事件
 */
class WidgetEnabledStateChangedEvent : public UIEvent {
 public:
  explicit WidgetEnabledStateChangedEvent(UUID widgetId, bool oldEnabled, bool newEnabled)
      : m_WidgetId(widgetId), m_OldEnabled(oldEnabled), m_NewEnabled(newEnabled)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  bool GetOldEnabledState() const
  {
    return m_OldEnabled;
  }
  bool GetNewEnabledState() const
  {
    return m_NewEnabled;
  }

  std::string ToString() const override
  {
    return "WidgetEnabledStateChangedEvent: ID " + UUIDGenerator::UUIDToString(m_WidgetId) +
           " from " +
           (m_OldEnabled ? "enabled" : "disabled") + " to " +
           (m_NewEnabled ? "enabled" : "disabled");
  }

  Event *Clone() const override
  {
    return new WidgetEnabledStateChangedEvent(m_WidgetId, m_OldEnabled, m_NewEnabled);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
  bool m_OldEnabled;
  bool m_NewEnabled;
};

/**
 * @brief 布局更新请求事件
 */
class LayoutUpdateRequestEvent : public UIEvent {
 public:
  explicit LayoutUpdateRequestEvent(UUID widgetId = {}) : m_WidgetId(widgetId) {}

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  std::string ToString() const override
  {
    if (m_WidgetId.is_nil()) {
      return "LayoutUpdateRequestEvent: Global layout update requested";
    }
    return "LayoutUpdateRequestEvent: Layout update requested for widget ID " +
           UUIDGenerator::UUIDToString(m_WidgetId);
  }

  Event *Clone() const override
  {
    return new LayoutUpdateRequestEvent(m_WidgetId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
};

/**
 * @brief 布局完成事件
 */
class LayoutCompletedEvent : public UIEvent {
 public:
  explicit LayoutCompletedEvent(UUID widgetId) : m_WidgetId(widgetId) {}

  UUID GetSourceWidgetID() const override
  {
    return m_WidgetId;
  }

  std::string ToString() const override
  {
    if (m_WidgetId.is_nil()) {
      return "LayoutCompletedEvent: Global layout completed";
    }
    return "LayoutCompletedEvent: Layout completed for widget ID " +
           UUIDGenerator::UUIDToString(m_WidgetId);
  }

  Event *Clone() const override
  {
    return new LayoutCompletedEvent(m_WidgetId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  UUID m_WidgetId;
};

}  // namespace mite

#endif  // MITE_UI_LAYOUT_EVENTS_H
