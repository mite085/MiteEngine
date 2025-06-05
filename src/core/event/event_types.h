#ifndef MITE_EVENT_TYPES
#define MITE_EVENT_TYPES

namespace mite {

// 事件类别，支持掩码判断，
// 如 flags = EventCategoryInput | EventCategoryKeyboard;
// 表示flags既是输入事件又是键盘事件。
// 使用flags & (EventCategoryInput | EventCategoryKeyboard) != 0,判断类别是否符合
enum EventCategory {
  None = 0,
  EventCategoryWindow = 1 << 0,
  EventCategoryInput = 1 << 1,
  EventCategoryKeyboard = 1 << 2,
  EventCategoryMouse = 1 << 3,
  EventCategoryEditor = 1 << 4,  // 编辑器特定事件分类
  EventCategoryCustom = 1 << 5   // 用户自定义事件分类
};

// 事件类型
enum class EventType {
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,

  KeyPressed,
  KeyReleased,
  KeyTyped,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,

  AssetLoaded,      // 资产加载
  MaterialChanged,  // 材质变更
  EntitySelected,   // 编辑器特定事件

  CustomEvent  // 用户自定义事件
};

}  // namespace mite

#endif
