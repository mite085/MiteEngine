#ifndef MITE_UI_EVENT_TYPES_H
#define MITE_UI_EVENT_TYPES_H

#include "event/event_types.h"

namespace mite {

// UI事件类别扩展
enum UIEventCategory: uint64_t {
  UI_EVENT_CATEGORY_BASE = 1 << 16,         // UI基础事件类别
  UI_EVENT_CATEGORY_INTERACTION = 1 << 17,  // 用户交互事件
  UI_EVENT_CATEGORY_LIFECYCLE = 1 << 18,    // UI生命周期事件
  UI_EVENT_CATEGORY_LAYOUT = 1 << 19,       // 布局相关事件
  UI_EVENT_CATEGORY_STYLE = 1 << 20,        // 样式相关事件
  UI_EVENT_CATEGORY_EDITOR = 1 << 21,       // 编辑器UI事件
  UI_EVENT_CATEGORY_RUNTIME = 1 << 22       // 运行时UI事件
};

// UI事件类型枚举
enum class UIEventType {
  // 基础UI事件 =====================================================
  UI_INITIALIZED,  // UI系统初始化完成
  UI_SHUTDOWN,     // UI系统关闭
  UI_BEGIN_FRAME,  // UI帧开始
  UI_END_FRAME,    // UI帧结束

  // 交互事件 =======================================================
  UI_MOUSE_ENTER,  // 鼠标进入控件区域
  UI_MOUSE_LEAVE,  // 鼠标离开控件区域
  UI_MOUSE_HOVER,  // 鼠标悬停在控件上

  // 控件交互事件基类（具体控件事件从此开始）=========================
  UI_BUTTON_CLICKED,      // 按钮点击
  UI_SLIDER_CHANGED,      // 滑块值改变
  UI_CHECKBOX_TOGGLED,    // 复选框状态切换
  UI_TEXT_INPUT_CHANGED,  // 文本输入内容改变
  UI_COMBOBOX_SELECTED,   // 下拉框选择项改变

  // 布局事件 =======================================================
  UI_LAYOUT_CHANGED,  // 布局发生变化
  UI_WIDGET_RESIZED,  // 控件大小改变
  UI_WIDGET_MOVED,    // 控件位置改变

  // 样式事件 =======================================================
  UI_STYLE_CHANGED,  // 样式配置改变
  UI_THEME_CHANGED,  // 主题切换

  // 编辑器专用事件 =================================================
  UI_EDITOR_VIEWPORT_CLICKED,   // 视口点击
  UI_EDITOR_VIEWPORT_DRAGGED,   // 视口拖拽
  UI_EDITOR_VIEWPORT_ZOOMED,    // 视口缩放
  UI_EDITOR_ENTITY_SELECTED,    // 实体选择
  UI_EDITOR_COMPONENT_ADDED,    // 组件添加
  UI_EDITOR_COMPONENT_REMOVED,  // 组件移除
  UI_EDITOR_MATERIAL_EDITED,    // 材质编辑
  UI_EDITOR_ASSET_SELECTED,     // 资源选择
  UI_EDITOR_GIZMO_INTERACTION,  // Gizmo交互

  // 运行时专用事件 =================================================
  UI_RUNTIME_PAUSE,          // 运行时暂停
  UI_RUNTIME_RESUME,         // 运行时继续
  UI_RUNTIME_STATE_CHANGED,  // 运行时状态改变

  UI_CUSTOM_EVENT  // 用户自定义UI事件
};

}  // namespace mite

#endif  // MITE_UI_EVENT_TYPES_H
