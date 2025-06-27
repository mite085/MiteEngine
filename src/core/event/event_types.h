#ifndef MITE_EVENT_TYPES
#define MITE_EVENT_TYPES

namespace mite {

// 事件类别，支持掩码判断，
// 如 flags = EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD;
// 表示flags既是输入事件又是键盘事件。
// 使用flags & (EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD) != 0,判断类别是否符合
enum EventCategory {
  None = 0,
  EVENT_CATEGORY_WINDOW = 1 << 0,
  EVENT_CATEGORY_INPUT = 1 << 1,
  EVENT_CATEGORY_KEYBOARD = 1 << 2,
  EVENT_CATEGORY_MOUSE = 1 << 3,
  EVENT_CATEGORY_EDITOR = 1 << 4,			// 编辑器特定事件分类
  EVENT_CATEGORY_CUSTOM = 1 << 5,			// 用户自定义事件分类

  EVENT_CATEGORY_SCENE_CHANGE = 1 << 6,	// 场景变更事件分类
  EVENT_CATEGORY_RENDER = 1 << 7
};

// 事件类型
enum class EventType {
  None = 0,

  // EventCategoryWindow 窗口事件 ==================================
  WINDOW_CLOSE,
  WINDOW_RESIZE,
  WINDOW_FOCUS,
  WINDOW_LOST_FOCUS,
  WINDOW_MOVED,

  // EventCategoryKeyboard 键盘输入事件 ============================
  KEY_PRESSED,
  KEY_RELEASED,
  KEY_TYPED,

  // EventCategoryMouse 鼠标输入事件 ===============================
  MOUSE_BUTTON_PRESSED,
  MOUSE_BUTTON_RELEASED,
  MOUSE_POSITION_MOVED,
  MOUSE_SCROLLED,

  // EventCategoryEditor 编辑器事件 ================================
  ASSERT_LOADED,      // 资产加载
  MATERIAL_CHANGED,  // 材质变更

  // EventCategorySceneChange 场景变更事件 =========================
  ENTITY_CREATED,     // 实体创建
  ENTITY_DESTROYED,   // 实体销毁
  COMPONENT_ADDED,    // 组件添加
  COMPONENT_REMOVED,  // 组件移除
  COMPONENT_CHANGED,  // 组件修改
  PARENT_CHANGED,     // 父节点变化
  TAG_CHANGED,        // 标签变化
  SCENE_LOADED,       // 场景加载
  SCENE_CLEARED,       // 场景清空

  CustomEvent  // 用户自定义事件
};

}  // namespace mite

#endif
