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
  UIStyle();
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
  T GetProperty(const std::string &propertyName, const T &defaultValue = T()) const;

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

// 常用样式属性名称定义
namespace StyleProperties {
// 颜色相关
constexpr const char *COLOR_BACKGROUND = "color.background";
constexpr const char *COLOR_TEXT = "color.text";
constexpr const char *COLOR_BORDER = "color.border";
constexpr const char *COLOR_HOVER = "color.hover";
constexpr const char *COLOR_ACTIVE = "color.active";
constexpr const char *COLOR_DISABLED = "color.disabled";

// 尺寸相关
constexpr const char *SIZE_WIDTH = "size.width";
constexpr const char *SIZE_HEIGHT = "size.height";
constexpr const char *SIZE_MIN_WIDTH = "size.min_width";
constexpr const char *SIZE_MIN_HEIGHT = "size.min_height";
constexpr const char *SIZE_MAX_WIDTH = "size.max_width";
constexpr const char *SIZE_MAX_HEIGHT = "size.max_height";

// 边框相关
constexpr const char *BORDER_WIDTH = "border.width";
constexpr const char *BORDER_RADIUS = "border.radius";

// 间距相关
constexpr const char *PADDING_LEFT = "padding.left";
constexpr const char *PADDING_RIGHT = "padding.right";
constexpr const char *PADDING_TOP = "padding.top";
constexpr const char *PADDING_BOTTOM = "padding.bottom";
constexpr const char *MARGIN_LEFT = "margin.left";
constexpr const char *MARGIN_RIGHT = "margin.right";
constexpr const char *MARGIN_TOP = "margin.top";
constexpr const char *MARGIN_BOTTOM = "margin.bottom";

// 字体相关
constexpr const char *FONT_SIZE = "font.size";
constexpr const char *FONT_FAMILY = "font.family";

// 布局相关
constexpr const char *LAYOUT_ALIGN = "layout.align";
constexpr const char *LAYOUT_JUSTIFY = "layout.justify";
}  // namespace StyleProperties
}  // namespace mite

#endif  // MITE_UI_STYLE_H
