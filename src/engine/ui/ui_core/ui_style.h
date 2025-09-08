#ifndef MITE_UI_STYLE_H
#define MITE_UI_STYLE_H

#include "headers/headers.h"

namespace mite {
/**
 * @brief 样式值类型定义
 */
using StyleValue = std::variant<int,         // 整数值
                                float,       // 浮点数值
                                bool,        // 布尔值
                                glm::vec2,   // 二维向量
                                glm::vec3,   // 三维向量
                                glm::vec4,   // 四维向量
                                std::string  // 字符串值
                                >;

/**
 * @brief 样式属性定义
 */
struct StyleProperty {
  StyleValue value;
  std::string description;  // 属性描述
};

/**
 * @brief 样式类，管理UI控件的视觉样式
 */
class UIStyle {
 public:
  explicit UIStyle(const std::string &name);
  ~UIStyle();

  /**
   * @brief 设置样式属性
   * @param propertyName 属性名称
   * @param value 属性值
   * @param description 属性描述（可选）
   */
  void SetProperty(const std::string &propertyName,
                   const StyleValue &value,
                   const std::string &description = "");

  /**
   * @brief 获取样式属性
   * @param propertyName 属性名称
   * @return 属性值，如果不存在返回默认值
   */
  template<typename T>
  T GetProperty(const std::string &propertyName, const T &defaultValue = T()) const
  {
    auto it = m_Properties.find(propertyName);
    if (it != m_Properties.end()) {
      try {
        return std::get<T>(it->second.value);
      }
      catch (const std::bad_variant_access &) {
        LOG_WARN("Style property {} is not of requested type", propertyName);
      }
    }
    // 检查父样式
    if (m_Parent) {
      return m_Parent->GetProperty<T>(propertyName, defaultValue);
    }
    return defaultValue;
  }

  /**
   * @brief 检查属性是否存在
   * @param propertyName 属性名称
   */
  bool HasProperty(const std::string &propertyName) const;

  /**
   * @brief 移除样式属性
   * @param propertyName 属性名称
   */
  void RemoveProperty(const std::string &propertyName);

  /**
   * @brief 清空所有样式属性
   */
  void Clear();

  /**
   * @brief 合并另一个样式
   * @param other 要合并的样式
   * @param overwrite 是否覆盖现有属性
   */
  void Merge(const UIStyle &other, bool overwrite = true);

  /**
   * @brief 获取所有属性名称
   */
  std::vector<std::string> GetPropertyNames() const;

  /**
   * @brief 获取属性数量
   */
  size_t GetPropertyCount() const;

  /**
   * @brief 设置样式名称
   */
  void SetName(const std::string &name)
  {
    m_Name = name;
  }

  /**
   * @brief 获取样式名称
   */
  const std::string &GetName() const
  {
    return m_Name;
  }

  /**
   * @brief 设置父样式
   * @param parent 父样式共享指针
   */
  void SetParent(std::shared_ptr<UIStyle> parent)
  {
    m_Parent = parent;
  }

  /**
   * @brief 获取父样式
   * @return std::shared_ptr<UIStyle> 父样式共享指针
   */
  std::shared_ptr<UIStyle> GetParent() const
  {
    return m_Parent;
  }

 private:
  std::string m_Name;                                           // 样式名称
  std::unordered_map<std::string, StyleProperty> m_Properties;  // 属性存储
  std::shared_ptr<UIStyle> m_Parent;                            // 父样式指针
};

// 常用样式属性名称定义（基于imgui的声明编写）
namespace StyleProperties {
// 颜色相关
constexpr const char *COLOR_BACKGROUND = "color.background";
constexpr const char *COLOR_TEXT = "color.text";
constexpr const char *COLOR_BORDER = "color.border";
constexpr const char *COLOR_HOVER = "color.hover";
constexpr const char *COLOR_ACTIVE = "color.active";
constexpr const char *COLOR_DISABLED = "color.disabled";
constexpr const char *COLOR_HEADER = "color.header";
constexpr const char *COLOR_HEADER_HOVER = "color.header_hover";
constexpr const char *COLOR_HEADER_ACTIVE = "color.header_active";
constexpr const char *COLOR_FRAME_BG = "color.frame_bg";
constexpr const char *COLOR_FRAME_BG_HOVER = "color.frame_bg_hover";
constexpr const char *COLOR_FRAME_BG_ACTIVE = "color.frame_bg_active";
constexpr const char *COLOR_TITLE_BG = "color.title_bg";
constexpr const char *COLOR_TITLE_BG_ACTIVE = "color.title_bg_active";
constexpr const char *COLOR_TITLE_BG_COLLAPSED = "color.title_bg_collapsed";
constexpr const char *COLOR_MENU_BAR_BG = "color.menu_bar_bg";
constexpr const char *COLOR_SCROLLBAR_BG = "color.scrollbar_bg";
constexpr const char *COLOR_SCROLLBAR_GRAB = "color.scrollbar_grab";
constexpr const char *COLOR_SCROLLBAR_GRAB_HOVER = "color.scrollbar_grab_hover";
constexpr const char *COLOR_SCROLLBAR_GRAB_ACTIVE = "color.scrollbar_grab_active";
constexpr const char *COLOR_CHECK_MARK = "color.check_mark";
constexpr const char *COLOR_SLIDER_GRAB = "color.slider_grab";
constexpr const char *COLOR_SLIDER_GRAB_ACTIVE = "color.slider_grab_active";
constexpr const char *COLOR_BUTTON = "color.button";
constexpr const char *COLOR_BUTTON_HOVER = "color.button_hover";
constexpr const char *COLOR_BUTTON_ACTIVE = "color.button_active";
constexpr const char *COLOR_SEPARATOR = "color.separator";
constexpr const char *COLOR_SEPARATOR_HOVER = "color.separator_hover";
constexpr const char *COLOR_SEPARATOR_ACTIVE = "color.separator_active";
constexpr const char *COLOR_RESIZE_GRIP = "color.resize_grip";
constexpr const char *COLOR_RESIZE_GRIP_HOVER = "color.resize_grip_hover";
constexpr const char *COLOR_RESIZE_GRIP_ACTIVE = "color.resize_grip_active";
constexpr const char *COLOR_TAB = "color.tab";
constexpr const char *COLOR_TAB_HOVER = "color.tab_hover";
constexpr const char *COLOR_TAB_ACTIVE = "color.tab_active";
constexpr const char *COLOR_TAB_UNFOCUSED = "color.tab_unfocused";
constexpr const char *COLOR_TAB_UNFOCUSED_ACTIVE = "color.tab_unfocused_active";
constexpr const char *COLOR_PLOT_LINES = "color.plot_lines";
constexpr const char *COLOR_PLOT_LINES_HOVER = "color.plot_lines_hover";
constexpr const char *COLOR_PLOT_HISTOGRAM = "color.plot_histogram";
constexpr const char *COLOR_PLOT_HISTOGRAM_HOVER = "color.plot_histogram_hover";
constexpr const char *COLOR_TABLE_HEADER_BG = "color.table_header_bg";
constexpr const char *COLOR_TABLE_BORDER_STRONG = "color.table_border_strong";
constexpr const char *COLOR_TABLE_BORDER_LIGHT = "color.table_border_light";
constexpr const char *COLOR_TABLE_ROW_BG = "color.table_row_bg";
constexpr const char *COLOR_TABLE_ROW_BG_ALT = "color.table_row_bg_alt";
constexpr const char *COLOR_TEXT_SELECTED_BG = "color.text_selected_bg";
constexpr const char *COLOR_DRAG_DROP_TARGET = "color.drag_drop_target";
constexpr const char *COLOR_NAV_HIGHLIGHT = "color.nav_highlight";
constexpr const char *COLOR_NAV_WINDOWING_HIGHLIGHT = "color.nav_windowing_highlight";
constexpr const char *COLOR_NAV_WINDOWING_DIM_BG = "color.nav_windowing_dim_bg";
constexpr const char *COLOR_MODAL_WINDOW_DIM_BG = "color.modal_window_dim_bg";

// 尺寸相关
constexpr const char *SIZE_WINDOW_PADDING_X = "size.window_padding_x";
constexpr const char *SIZE_WINDOW_PADDING_Y = "size.window_padding_y";
constexpr const char *SIZE_WINDOW_ROUNDING = "size.window_rounding";
constexpr const char *SIZE_WINDOW_MIN_SIZE_X = "size.window_min_size_x";
constexpr const char *SIZE_WINDOW_MIN_SIZE_Y = "size.window_min_size_y";
constexpr const char *SIZE_WINDOW_TITLE_ALIGN_X = "size.window_title_align_x";
constexpr const char *SIZE_WINDOW_TITLE_ALIGN_Y = "size.window_title_align_y";
constexpr const char *SIZE_CHILD_ROUNDING = "size.child_rounding";
constexpr const char *SIZE_CHILD_BORDER_SIZE = "size.child_border_size";
constexpr const char *SIZE_POPUP_ROUNDING = "size.popup_rounding";
constexpr const char *SIZE_POPUP_BORDER_SIZE = "size.popup_border_size";
constexpr const char *SIZE_FRAME_PADDING_X = "size.frame_padding_x";
constexpr const char *SIZE_FRAME_PADDING_Y = "size.frame_padding_y";
constexpr const char *SIZE_FRAME_ROUNDING = "size.frame_rounding";
constexpr const char *SIZE_FRAME_BORDER_SIZE = "size.frame_border_size";
constexpr const char *SIZE_ITEM_SPACING_X = "size.item_spacing_x";
constexpr const char *SIZE_ITEM_SPACING_Y = "size.item_spacing_y";
constexpr const char *SIZE_ITEM_INNER_SPACING_X = "size.item_inner_spacing_x";
constexpr const char *SIZE_ITEM_INNER_SPACING_Y = "size.item_inner_spacing_y";
constexpr const char *SIZE_CELL_PADDING_X = "size.cell_padding_x";
constexpr const char *SIZE_CELL_PADDING_Y = "size.cell_padding_y";
constexpr const char *SIZE_TOUCH_EXTRA_PADDING_X = "size.touch_extra_padding_x";
constexpr const char *SIZE_TOUCH_EXTRA_PADDING_Y = "size.touch_extra_padding_y";
constexpr const char *SIZE_INDENT_SPACING = "size.indent_spacing";
constexpr const char *SIZE_COLUMNS_MIN_SPACING = "size.columns_min_spacing";
constexpr const char *SIZE_SCROLLBAR_SIZE = "size.scrollbar_size";
constexpr const char *SIZE_SCROLLBAR_ROUNDING = "size.scrollbar_rounding";
constexpr const char *SIZE_GRAB_MIN_SIZE = "size.grab_min_size";
constexpr const char *SIZE_GRAB_ROUNDING = "size.grab_rounding";
constexpr const char *SIZE_LOG_SLIDER_DEADZONE = "size.log_slider_deadzone";
constexpr const char *SIZE_TAB_ROUNDING = "size.tab_rounding";
constexpr const char *SIZE_TAB_BORDER_SIZE = "size.tab_border_size";
constexpr const char *SIZE_TAB_MIN_WIDTH_FOR_CLOSE_BUTTON = "size.tab_min_width_for_close_button";
constexpr const char *SIZE_BUTTON_TEXT_ALIGN_X = "size.button_text_align_x";
constexpr const char *SIZE_BUTTON_TEXT_ALIGN_Y = "size.button_text_align_y";
constexpr const char *SIZE_SELECTABLE_TEXT_ALIGN_X = "size.selectable_text_align_x";
constexpr const char *SIZE_SELECTABLE_TEXT_ALIGN_Y = "size.selectable_text_align_y";
constexpr const char *SIZE_DISPLAY_WINDOW_PADDING_X = "size.display_window_padding_x";
constexpr const char *SIZE_DISPLAY_WINDOW_PADDING_Y = "size.display_window_padding_y";
constexpr const char *SIZE_DISPLAY_SAFE_AREA_PADDING_X = "size.display_safe_area_padding_x";
constexpr const char *SIZE_DISPLAY_SAFE_AREA_PADDING_Y = "size.display_safe_area_padding_y";
constexpr const char *SIZE_MOUSE_CURSOR_SCALE = "size.mouse_cursor_scale";
constexpr const char *SIZE_ANTI_ALIASED_LINES = "size.anti_aliased_lines";
constexpr const char *SIZE_ANTI_ALIASED_LINES_USE_TEX = "size.anti_aliased_lines_use_tex";
constexpr const char *SIZE_ANTI_ALIASED_FILL = "size.anti_aliased_fill";
constexpr const char *SIZE_CURVE_TESSELLATION_TOL = "size.curve_tessellation_tol";
constexpr const char *SIZE_CIRCLE_TESSELLATION_MAX_ERROR = "size.circle_tessellation_max_error";

// 布局相关（枚举类型）
constexpr const char *LAYOUT_COLOR_BUTTON_POSITION = "layout.color_button_position";
constexpr const char *LAYOUT_WINDOW_MENU_BUTTON_POSITION = "layout.window_menu_button_position";

// 边框相关
constexpr const char *BORDER_WINDOW = "border.window";
constexpr const char *BORDER_CHILD = "border.child";
constexpr const char *BORDER_POPUP = "border.popup";
constexpr const char *BORDER_FRAME = "border.frame";
constexpr const char *BORDER_TAB = "border.tab";

// 间距相关
constexpr const char *SPACING_WINDOW_PADDING_X = "spacing.window_padding_x";
constexpr const char *SPACING_WINDOW_PADDING_Y = "spacing.window_padding_y";
constexpr const char *SPACING_FRAME_PADDING_X = "spacing.frame_padding_x";
constexpr const char *SPACING_FRAME_PADDING_Y = "spacing.frame_padding_y";
constexpr const char *SPACING_ITEM_SPACING_X = "spacing.item_spacing_x";
constexpr const char *SPACING_ITEM_SPACING_Y = "spacing.item_spacing_y";
constexpr const char *SPACING_ITEM_INNER_SPACING_X = "spacing.item_inner_spacing_x";
constexpr const char *SPACING_ITEM_INNER_SPACING_Y = "spacing.item_inner_spacing_y";
constexpr const char *SPACING_CELL_PADDING_X = "spacing.cell_padding_x";
constexpr const char *SPACING_CELL_PADDING_Y = "spacing.cell_padding_y";
constexpr const char *SPACING_TOUCH_EXTRA_PADDING_X = "spacing.touch_extra_padding_x";
constexpr const char *SPACING_TOUCH_EXTRA_PADDING_Y = "spacing.touch_extra_padding_y";
constexpr const char *SPACING_INDENT_SPACING = "spacing.indent_spacing";
constexpr const char *SPACING_COLUMNS_MIN_SPACING = "spacing.columns_min_spacing";
}  // namespace StyleProperties

}  // namespace mite

#endif  // MITE_UI_STYLE_H
