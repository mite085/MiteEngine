#ifndef MITE_UI_RENDER_PROPS_H
#define MITE_UI_RENDER_PROPS_H

#include "headers/headers.h"

namespace mite {
// ==================== 基础属性结构 ====================

/**
 * @brief 基础渲染属性
 * 所有控件渲染属性的基类，包含位置、尺寸、翻译等基本信息
 */
struct BaseRenderProps {
  bool visible = true;  // 是否可见
  bool enabled = true;  // 是否启用

  std::string translationKey;  // 翻译键（用于查找本地化文本）
  std::string tooltip;         // 提示文本
};

/**
 * @brief 文本相关属性
 * 包含文本显示需求的控件基础属性
 */
struct TextRenderProps : public BaseRenderProps {
  // 文本相关属性已整合到BaseRenderProps中
  // 此结构主要用于类型标识
};

// ==================== 菜单控件属性 ====================
/**
 * @brief 菜单项属性
 * 用于渲染菜单项的基础属性
 */
struct MenuItemProps : public TextRenderProps {
  bool hasSubmenu = false;         // 是否有子菜单（枝干节点）
  bool isChecked = false;          // 是否被选中（用于复选框菜单项）
  bool isSeparator = false;        // 是否为分隔符
  std::string shortcut;            // 快捷键显示文本
  std::function<void()> callback;  // 点击回调（仅叶子节点有效）

  // 子菜单渲染回调（仅当hasSubmenu为true时有效）
  std::function<void()> submenuRenderCallback;
};
/**
 * @brief 菜单栏属性
 * 用于渲染菜单栏的基础属性
 */
struct MenuBarProps : public BaseRenderProps {
  // 菜单栏不需要额外属性，继承基础属性即可
  // 菜单栏内容由UIMenu直接渲染，不需要回调
};

// ==================== 基础控件属性 ====================

/**
 * @brief 标签控件属性
 * 用于显示静态文本内容
 */
struct LabelProps : public TextRenderProps {
  bool wordWrap = false;  // 是否自动换行
  float maxWidth = 0.0f;  // 最大宽度（0表示无限制）
};

/**
 * @brief 按钮控件属性
 * 可点击的按钮控件
 */
struct ButtonProps : public TextRenderProps {
  // 按钮的文本通过translationKey管理
  glm::vec2 size = {0, 0};
};

/**
 * @brief 复选框属性
 * 二选一选择控件
 */
struct CheckboxProps : public TextRenderProps {
  bool checked = false;  // 是否选中
};

/**
 * @brief 文本输入框属性
 * 单行文本输入控件
 */
struct TextInputProps : public TextRenderProps {
  std::string text;                // 当前文本内容
  std::string hintTranslationKey;  // 提示文本的翻译键
  size_t maxLength = 256;          // 最大输入长度
};

/**
 * @brief 多行文本输入属性
 * 支持多行文本输入
 */
struct TextAreaProps : public TextInputProps {
  int lineCount = 3;  // 显示行数
  glm::vec2 size = {0, 0};
};

// ==================== 选择器控件属性 ====================

/**
 * @brief 下拉选择框属性
 * 包含标签文本的下拉选择控件
 */
struct ComboboxProps : public TextRenderProps {
  std::vector<std::string> itemTranslationKeys;  // 选项的翻译键（下拉框所有内容）
  int selectedIndex = -1;                        // 当前选中索引
};

/**
 * @brief 列表框属性
 * 显示可选列表的控件
 */
struct ListBoxProps : public TextRenderProps {
  std::vector<std::string> itemTranslationKeys;  // 列表项的翻译键
  int selectedIndex = -1;                        // 选中项索引
  float itemHeight = 20.0f;                      // 单项高度
  glm::vec2 size = {0, 0};
};

// ==================== 数值输入控件属性 ====================

/**
 * @brief 浮点数编辑属性
 */
struct FloatEditProps : public TextRenderProps {
  float value = 0.0f;           // 当前值
  float dragSpeed = 0.1f;       // 拖动速度（Drag控件专用）
  float minValue = -FLT_MAX;    // 最小值
  float maxValue = FLT_MAX;     // 最大值
  std::string format = "%.3f";  // 显示格式
};

/**
 * @brief 二维向量编辑属性
 */
struct Float2EditProps : public TextRenderProps {
  glm::vec2 value = {0.0f, 0.0f};  // 当前值
  float dragSpeed = 0.1f;          // 拖动速度（Drag控件专用）
  float minValue = -FLT_MAX;       // 最小值
  float maxValue = FLT_MAX;        // 最大值
  std::string format = "%.3f";     // 显示格式
};

/**
 * @brief 三维向量编辑属性
 */
struct Float3EditProps : public TextRenderProps {
  glm::vec3 value = {0.0f, 0.0f, 0.0f};  // 当前值
  float dragSpeed = 0.1f;                // 拖动速度（Drag控件专用）
  float minValue = -FLT_MAX;             // 最小值
  float maxValue = FLT_MAX;              // 最大值
  std::string format = "%.3f";           // 显示格式
};

/**
 * @brief 四维向量编辑属性
 */
struct Float4EditProps : public TextRenderProps {
  glm::vec4 value = {0.0f, 0.0f, 0.0f, 0.0f};  // 当前值
  float dragSpeed = 0.1f;                      // 拖动速度（Drag控件专用）
  float minValue = -FLT_MAX;                   // 最小值
  float maxValue = FLT_MAX;                    // 最大值
  std::string format = "%.3f";                 // 显示格式
};

/**
 * @brief 整数编辑属性
 */
struct IntEditProps : public TextRenderProps {
  int value = 0;              // 当前值
  float dragSpeed = 1.0f;     // 拖动速度（Drag控件专用）
  int minValue = INT_MIN;     // 最小值
  int maxValue = INT_MAX;     // 最大值
  std::string format = "%d";  // 显示格式
};

// ==================== 特殊控件属性 ====================

/**
 * @brief 进度条属性
 * 显示进度信息的控件
 */
struct ProgressBarProps : public TextRenderProps {
  float progress = 0.0f;              // 进度值（0.0-1.0）
  std::string overlayTranslationKey;  // 覆盖文本的翻译键
  glm::vec2 size = {0, 0};
};

/**
 * @brief 颜色选择器属性
 * 用于选择颜色的控件
 */
struct ColorEditProps : public TextRenderProps {
  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};  // RGBA颜色值
  bool showAlpha = true;                       // 是否显示透明度通道
  bool showInputs = true;                      // 是否显示输入框
  bool showPicker = true;                      // 是否显示选择器
  bool showPreview = true;                     // 是否显示预览
  bool showTooltip = true;                     // 是否显示工具提示
};

/**
 * @brief 图像显示属性
 * 用于显示纹理图像的控件
 */
struct ImageProps : public TextRenderProps {
  uintptr_t textureId = 0;       // 纹理ID（后端相关）
  glm::vec2 size = {0, 0};       // 纹理尺寸
  glm::vec2 uv0 = {0.0f, 0.0f};  // UV坐标起始点
  glm::vec2 uv1 = {1.0f, 1.0f};  // UV坐标结束点
};

// ==================== 容器控件属性 ====================

/**
 * @brief 分组控件属性
 * 用于将相关控件分组显示
 */
struct GroupProps : public TextRenderProps {
  bool showBorder = true;  // 是否显示边框
};

/**
 * @brief 树节点属性
 * 用于构建树状结构的节点控件
 */
struct TreeNodeProps : public TextRenderProps {
  void *nodePtr = nullptr;  // 节点指针
  bool isSelect = false;    // 节点是否被选中
  bool isLeaf = false;      // 是否为叶子节点
  int depth = 0;            // 节点深度
};

/**
 * @brief 弹出窗口属性
 * 模态或非模态弹出窗口
 */
struct PopupProps : public TextRenderProps {
  bool modal = false;  // 是否为模态窗口
  bool open = false;   // 窗口是否打开
};

/**
 * @brief 表格属性
 * 用于显示表格数据的容器
 */
struct TableProps : public TextRenderProps {
  int columns = 1;                                 // 列数
  std::vector<std::string> headerTranslationKeys;  // 表头翻译键
  float rowHeight = 25.0f;                         // 行高

  // 基础特性
  bool showHeaders = true;  // 是否显示表头（默认显示）
  bool resizable = true;    // 是否可调整列宽（默认可调整）
  bool reorderable = true;  // 是否可重新排序列（默认支持）
  bool hideable = true;     // 是否可隐藏列（默认支持）
  bool sortable = false;    // 是否可排序（默认不支持）

  // 装饰选项
  bool rowBg = true;          // 是否显示行背景色（默认显示）
  bool borders = true;        // 是否显示边框（默认显示）
  bool bordersInnerH = true;  // 是否显示内部水平边框（默认显示）
  bool bordersInnerV = true;  // 是否显示内部垂直边框（默认显示）
  bool bordersOuterH = true;  // 是否显示外部水平边框（默认显示）
  bool bordersOuterV = true;  // 是否显示外部垂直边框（默认显示）

  // 尺寸策略
  enum SizingPolicy {
    SizingFixedFit,                    // 列宽适应内容
    SizingFixedSame,                   // 列宽相同
    SizingStretchProp,                 // 按比例拉伸
    SizingStretchSame                  // 等比例拉伸
  } sizingPolicy = SizingStretchProp;  // 默认按照比例拉伸

  // 尺寸额外选项
  bool noHostExtendX = false;  // 是否限制表格宽度（默认自动扩展）
  bool noHostExtendY = false;  // 是否限制表格高度（默认自动扩展）
  bool preciseWidths = false;  // 是否精确宽度分配（默认不精确）

  // 滚动选项
  bool scrollX = false;  // 是否启用水平滚动（默认不启用）
  bool scrollY = false;  // 是否启用垂直滚动（默认启用）

  // 填充选项
  bool padOuterX = false;    // 是否启用外部X填充（默认启用）
  bool noPadInnerX = false;  // 是否禁用内部X填充（默认不禁用）
};

// ==================== 布局控件属性 ====================

/**
 * @brief 空白间隔属性
 * 用于布局 spacing 的空白控件
 */
struct SpacerProps : public BaseRenderProps {
  glm::vec2 size = {0, 0};
};

// ==================== 面板控件属性 ====================
/**
 * @brief 面板属性
 * 用于创建和管理面板窗口
 */
struct PanelProps : public BaseRenderProps {
  // ============ 基本属性 ============
  bool resizable = true;               // 是否可调整大小（默认可调整）
  bool scrollable = true;              // 是否可滚动（默认可滚动）
  bool collapsed = false;              // 是否折叠标题（默认状态下不折叠）
  bool bringToFront = false;           // 是否置顶（默认状态下不置顶）
  bool dockable = true;                // 是否可停靠（默认支持）
  bool hasMenuBar = false;             // 是否有菜单栏（默认没有，使用统一的window菜单栏）
  bool noBackground = false;           // 无背景
  glm::vec2 minSize = {10, 10};        // 最小尺寸
  glm::vec2 maxSize = {10000, 10000};  // 最大尺寸

  // ============ 运行时状态 ============
  bool movable = true;     // 可移动flag，当鼠标移入显示区域时自动设为false
  bool isFocused = false;  // 是否聚焦于此Panel
  bool isHovered = false;  // 鼠标是否悬浮于此Panel上
};

struct ChildProps : public BaseRenderProps {
  // ============ 子窗口基本属性 ============
  glm::vec2 size = {0, 0};                 // 固定尺寸（0表示自动）
  bool border = true;                      // 是否显示边框
  bool autoResizeX = true;                 // X轴自动调整大小
  bool autoResizeY = true;                 // Y轴自动调整大小
  bool alwaysVerticalScrollbar = false;    // 总是显示垂直滚动条
  bool alwaysHorizontalScrollbar = false;  // 总是显示水平滚动条
  bool scrollable = true;                  // 是否可滚动
  bool noBackground = false;               // 无背景

  // ============ 运行时状态 ============
  bool isHovered = false;  // 是否悬停
};
}  // namespace mite

#endif  // MITE_UI_RENDER_PROPS_H
