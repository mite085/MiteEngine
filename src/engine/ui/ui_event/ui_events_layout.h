#ifndef MITE_UI_LAYOUT_EVENTS_H
#define MITE_UI_LAYOUT_EVENTS_H

#include "ui_event.h"

namespace mite {

/**
 * @brief 控件位置改变事件
 */
class ElementPositionChangedEvent : public UIEvent {
 public:
  explicit ElementPositionChangedEvent(UUID elementId,
                                      const glm::vec2 &oldPosition,
                                      const glm::vec2 &newPosition)
      : UIEvent(elementId), m_OldPosition(oldPosition), m_NewPosition(newPosition)
  {
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
    return "ElementPositionChangedEvent: ID " + UUIDGenerator::UUIDToString(m_ElementId) +
           " from (" +
           std::to_string(m_OldPosition.x) + ", " + std::to_string(m_OldPosition.y) + ") to (" +
           std::to_string(m_NewPosition.x) + ", " + std::to_string(m_NewPosition.y) + ")";
  }

  Event *Clone() const override
  {
    return new ElementPositionChangedEvent(m_ElementId, m_OldPosition, m_NewPosition);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  glm::vec2 m_OldPosition;
  glm::vec2 m_NewPosition;
};

/**
 * @brief 控件大小改变事件
 */
class ElementSizeChangedEvent : public UIEvent {
 public:
  explicit ElementSizeChangedEvent(UUID elementId,
                                  const glm::vec2 &oldSize,
                                  const glm::vec2 &newSize)
      : UIEvent(elementId), m_OldSize(oldSize), m_NewSize(newSize)
  {
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
    return "ElementSizeChangedEvent: ID " + UUIDGenerator::UUIDToString(m_ElementId) + " from " +
           std::to_string(m_OldSize.x) + "x" + std::to_string(m_OldSize.y) + " to " +
           std::to_string(m_NewSize.x) + "x" + std::to_string(m_NewSize.y);
  }

  Event *Clone() const override
  {
    return new ElementSizeChangedEvent(m_ElementId, m_OldSize, m_NewSize);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  glm::vec2 m_OldSize;
  glm::vec2 m_NewSize;
};

/**
 * @brief 控件可见性改变事件
 */
class ElementVisibilityChangedEvent : public UIEvent {
 public:
  explicit ElementVisibilityChangedEvent(UUID elementId, bool oldVisible, bool newVisible)
      : UIEvent(elementId), m_OldVisible(oldVisible), m_NewVisible(newVisible)
  {
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
    return "ElementVisibilityChangedEvent: ID " + UUIDGenerator::UUIDToString(m_ElementId) +
           " from " +
           (m_OldVisible ? "visible" : "hidden") + " to " + (m_NewVisible ? "visible" : "hidden");
  }

  Event *Clone() const override
  {
    return new ElementVisibilityChangedEvent(m_ElementId, m_OldVisible, m_NewVisible);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  bool m_OldVisible;
  bool m_NewVisible;
};

/**
 * @brief 控件启用状态改变事件
 */
class ElementEnabledStateChangedEvent : public UIEvent {
 public:
  explicit ElementEnabledStateChangedEvent(UUID elementId, bool oldEnabled, bool newEnabled)
      : UIEvent(elementId), m_OldEnabled(oldEnabled), m_NewEnabled(newEnabled)
  {
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
    return "ElementEnabledStateChangedEvent: ID " + UUIDGenerator::UUIDToString(m_ElementId) +
           " from " +
           (m_OldEnabled ? "enabled" : "disabled") + " to " +
           (m_NewEnabled ? "enabled" : "disabled");
  }

  Event *Clone() const override
  {
    return new ElementEnabledStateChangedEvent(m_ElementId, m_OldEnabled, m_NewEnabled);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

 private:
  bool m_OldEnabled;
  bool m_NewEnabled;
};

/**
 * @brief 控件悬停事件
 */
class ElementHoverStateChangedEvent : public UIEvent {
 public:
  explicit ElementHoverStateChangedEvent(UUID elementId, bool hovered)
      : UIEvent(elementId), m_Hovered(hovered)
  {
  }

  std::string ToString() const override
  {
    return "ElementHoverStateChangedEvent: " + std::string(m_Hovered ? "enabled" : "disabled");
  }

  Event *Clone() const override
  {
    return new ElementHoverStateChangedEvent(m_ElementId, m_Hovered);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)
 private:
  bool m_Hovered;
};

/**
 * @brief 布局更新请求事件
 */
class LayoutUpdateRequestEvent : public UIEvent {
 public:
  explicit LayoutUpdateRequestEvent(UUID elementId = {}) : UIEvent(elementId) {}

  std::string ToString() const override
  {
    if (m_ElementId.is_nil()) {
      return "LayoutUpdateRequestEvent: Global layout update requested";
    }
    return "LayoutUpdateRequestEvent: Layout update requested for widget ID " +
           UUIDGenerator::UUIDToString(m_ElementId);
  }

  Event *Clone() const override
  {
    return new LayoutUpdateRequestEvent(m_ElementId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)

};

/**
 * @brief 布局完成事件
 */
class LayoutCompletedEvent : public UIEvent {
 public:
  explicit LayoutCompletedEvent(UUID elementId = {}) : UIEvent(elementId) {}

  std::string ToString() const override
  {
    if (m_ElementId.is_nil()) {
      return "LayoutCompletedEvent: Global layout completed";
    }
    return "LayoutCompletedEvent: Layout completed for widget ID " +
           UUIDGenerator::UUIDToString(m_ElementId);
  }

  Event *Clone() const override
  {
    return new LayoutCompletedEvent(m_ElementId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_LAYOUT)
};



}  // namespace mite

#endif  // MITE_UI_LAYOUT_EVENTS_H
