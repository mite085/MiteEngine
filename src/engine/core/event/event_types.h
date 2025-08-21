#ifndef MITE_EVENT_TYPES
#define MITE_EVENT_TYPES

namespace mite {
// 事件类别，支持掩码判断，
// 如 flags = EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD;
// 表示flags既是输入事件又是键盘事件。
// 使用flags & (EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD) != 0,判断类别是否符合
// （键盘鼠标分开意义不大，合并到EVENT_CATEGORY_INPUT了）
enum EventCategory {
  None = 0,
  EVENT_CATEGORY_WINDOW = 1 << 0,    // 窗口事件
  EVENT_CATEGORY_INPUT = 1 << 1,     // 键盘/鼠标输入事件
  EVENT_CATEGORY_EDITOR = 1 << 4,    // 编辑器特定事件
  EVENT_CATEGORY_CUSTOM = 1 << 5,    // 用户自定义事件

  EVENT_CATEGORY_SCENE_CHANGE = 1 << 6,  // 场景变更事件
  EVENT_CATEGORY_RENDER = 1 << 7,        // 渲染事件
  EVENT_CATEGORY_ASSET = 1 << 8,         // 资产事件
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

  // EVENT_CATEGORY_INPUT 键盘鼠标输入事件 ============================
  KEY_PRESSED,
  KEY_RELEASED,
  KEY_TYPED,
  MOUSE_BUTTON_PRESSED,
  MOUSE_BUTTON_RELEASED,
  MOUSE_POSITION_MOVED,
  MOUSE_SCROLLED,

  // EventCategoryEditor 编辑器事件 ================================
  ASSERT_LOADED,     // 资产加载
  MATERIAL_CHANGED,  // 材质变更

  // EventCategorySceneChange 场景变更事件 =========================
  ENTITY_CREATED,     // 实体创建
  ENTITY_DESTROYED,   // 实体销毁

  COMPONENT_ADDED_EVENT_BASE = 0x10000,  // 组件添加事件区间（自动生成，预留高位区间）
  COMPONENT_REMOVED_EVENT_BASE = 0x20000,  // 组件移除事件区间（自动生成，预留高位区间）

  TAG_CHANGED,    // 标签变化
  SCENE_LOADED,   // 场景加载
  SCENE_CLEARED,  // 场景清空

  MODEL_LOADED,    // 模型加载
  TEXTURE_LOADED,  // 贴图加载

  VISIBILITY_COMPONENT_CHANGED,  // 可见性组件修改

  MATERIAL_COMPONENT_CHANGED,  // 材质组件修改
  MESH_COMPONENT_CHANGED,      // 网格组件修改

  HIERACHY_COMPONENT_PARENT_CHANGED,  // 父节点变化
  HIERACHY_COMPONENT_CHILD_ADDED,     // 子节点添加
  HIERACHY_COMPONENT_CHILD_REMOVE,    // 子节点删除

  TRANSFORM_COMPONENT_UPDATE,             // 旋转矩阵组件替换
  TRANSFORM_COMPONENT_POSITION_CHANGED,   // 位置变换
  TRANSFORM_COMPONENT_ROTATION_CHANGED,   // 旋转变换
  TRANSFORM_COMPONENT_SCALE_CHANGED,      // 缩放变换
  TRANSFORM_COMPONENT_TRANSFORM_CHANGED,  // 矩阵变换

  CustomEvent  // 用户自定义事件
};
}  // namespace mite

#endif
