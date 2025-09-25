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
  UUID elementId;       // 控件唯一标识
  bool visible = true;  // 是否可见
  bool enabled = true;  // 是否启用

  std::string translationKey;  // 翻译键（用于查找本地化文本）
  std::string fallbackText;    // 回退文本（翻译键查找失败时使用）
};

/**
 * @brief 文本相关属性
 * 包含文本显示需求的控件基础属性
 */
struct TextRenderProps : public BaseRenderProps {
  // 文本相关属性已整合到BaseRenderProps中
  // 此结构主要用于类型标识
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
  // 按钮的文本通过translationKey和fallbackText管理
  glm::uvec2 size = {0, 0};
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
  std::string hintFallbackText;    // 提示文本的回退文本
  size_t maxLength = 256;          // 最大输入长度
  bool isPassword = false;         // 是否为密码输入
};

/**
 * @brief 多行文本输入属性
 * 支持多行文本输入
 */
struct TextAreaProps : public TextInputProps {
  int lineCount = 3;  // 显示行数
  glm::uvec2 size = {0, 0};
};

// ==================== 选择器控件属性 ====================

/**
 * @brief 下拉选择框属性
 * 包含标签文本的下拉选择控件
 */
struct ComboboxProps : public TextRenderProps {
  std::vector<std::string> items;                // 选项列表
  std::vector<std::string> itemTranslationKeys;  // 选项的翻译键
  int selectedIndex = -1;                        // 当前选中索引
  std::string previewText;                       // 预览文本
};

/**
 * @brief 列表框属性
 * 显示可选列表的控件
 */
struct ListBoxProps : public TextRenderProps {
  std::vector<std::string> items;                // 列表项
  std::vector<std::string> itemTranslationKeys;  // 列表项的翻译键
  int selectedIndex = -1;                        // 选中项索引
  float itemHeight = 20.0f;                      // 单项高度
  glm::uvec2 size = {0, 0};
};

// ==================== 数值输入控件属性 ====================

/**
 * @brief 浮点数拖动输入属性
 * 包含标签文本的数值输入控件
 */
struct DragFloatProps : public TextRenderProps {
  float value = 0.0f;           // 当前值
  float speed = 1.0f;           // 拖动速度
  float minValue = 0.0f;        // 最小值
  float maxValue = 100.0f;      // 最大值
  std::string format = "%.3f";  // 显示格式
};

/**
 * @brief 二维向量拖动属性
 */
struct DragFloat2Props : public TextRenderProps {
  glm::vec2 value = {0.0f, 0.0f};  // 当前值
  float speed = 1.0f;              // 拖动速度
  float minValue = 0.0f;           // 最小值
  float maxValue = 100.0f;         // 最大值
  std::string format = "%.3f";     // 显示格式
};

/**
 * @brief 三维向量拖动属性
 */
struct DragFloat3Props : public TextRenderProps {
  glm::vec3 value = {0.0f, 0.0f, 0.0f};  // 当前值
  float speed = 1.0f;                    // 拖动速度
  float minValue = 0.0f;                 // 最小值
  float maxValue = 100.0f;               // 最大值
  std::string format = "%.3f";           // 显示格式
};

/**
 * @brief 四维向量拖动属性
 */
struct DragFloat4Props : public TextRenderProps {
  glm::vec4 value = {0.0f, 0.0f, 0.0f, 0.0f};  // 当前值
  float speed = 1.0f;                          // 拖动速度
  float minValue = 0.0f;                       // 最小值
  float maxValue = 100.0f;                     // 最大值
  std::string format = "%.3f";                 // 显示格式
};

/**
 * @brief 整数拖动属性
 */
struct DragIntProps : public TextRenderProps {
  int value = 0;              // 当前值
  float speed = 1.0f;         // 拖动速度
  int minValue = 0;           // 最小值
  int maxValue = 100;         // 最大值
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
  std::string overlayFallbackText;    // 覆盖文本的回退文本
  glm::uvec2 size = {0, 0};
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
  glm::uvec2 size = {0, 0};      // 纹理尺寸
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
  bool isOpen = false;       // 节点是否展开
  int depth = 0;             // 节点深度
  bool hasChildren = false;  // 是否有子节点
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
  std::vector<std::string> headerFallbackTexts;    // 表头回退文本
  bool showHeaders = true;                         // 是否显示表头
  float rowHeight = 25.0f;                         // 行高
};

// ==================== 布局控件属性 ====================

/**
 * @brief 空白间隔属性
 * 用于布局 spacing 的空白控件
 */
struct SpacerProps : public BaseRenderProps {
  glm::uvec2 size = {0, 0};
};

// ==================== 面板控件属性 ====================
/**
 * @brief 面板属性
 * 用于创建和管理面板窗口
 */
struct PanelProps : public BaseRenderProps {
  bool movable = true;                 // 是否可移动
  bool resizable = true;               // 是否可调整大小
  bool scrollable = true;              // 是否可滚动
  bool collapsed = false;              // 是否折叠标题
  bool bringToFront = false;           // 是否置顶
  glm::vec2 minSize = {100, 100};      // 最小尺寸
  glm::vec2 maxSize = {10000, 10000};  // 最大尺寸

  // ============ Dock相关属性 ============
  bool dockable = true;                                    // 是否可停靠
};
/**
 * @brief 子窗口属性
 * 用于创建子窗口区域
 */
struct ChildProps : public BaseRenderProps {
  bool border = false;      // 是否显示边框
  glm::vec2 size = {0, 0};  // 子窗口尺寸（0表示自动）
};

}  // namespace mite

#endif  // MITE_UI_RENDER_PROPS_H
