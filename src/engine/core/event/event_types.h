#ifndef MITE_EVENT_TYPES
#define MITE_EVENT_TYPES

#include <stdint.h>

namespace mite {
// 事件类别，支持掩码判断，
// 如 flags = EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD;
// 表示flags既是输入事件又是键盘事件。
// 使用flags & (EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD) != 0,判断类别是否符合
// （键盘鼠标分开意义不大，合并到EVENT_CATEGORY_INPUT了）
enum EventCategory : uint64_t {
  None = 0,
  EVENT_CATEGORY_WINDOW = 1 << 0,  // 窗口事件
  EVENT_CATEGORY_INPUT = 1 << 1,   // 键盘/鼠标输入事件

  EVENT_CATEGORY_SCENE_CHANGE = 1 << 6,    // 场景变更事件
  EVENT_CATEGORY_RENDER = 1 << 7,          // 渲染事件
  EVENT_CATEGORY_ASSET = 1 << 8,           // 资产事件

  UI_EVENT_CATEGORY_INTERACTION = 1 << 9,  // 界面：基础交互事件
  UI_EVENT_CATEGORY_LIFECYCLE = 1 << 10,   // 界面：生命周期事件
  UI_EVENT_CATEGORY_EDITOR = 1 << 11,      // 界面：编辑器专用事件
  UI_EVENT_CATEGORY_RUNTIME = 1 << 12,     // 界面：运行时专用事件
};
}  // namespace mite

#endif
