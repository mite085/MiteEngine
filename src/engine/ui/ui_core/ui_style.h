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
   */
  void SetParent(std::shared_ptr<UIStyle> parent)
  {
    m_Parent = parent;
  }

  /**
   * @brief 获取父样式
   */
  std::shared_ptr<UIStyle> GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief 应用默认样式
   */
  void ApplyDefaultStyle();

 private:
  std::string m_Name;
  std::unordered_map<std::string, StyleProperty> m_Properties;
  std::shared_ptr<UIStyle> m_Parent;
};

// 显式实例化模板特化
extern template int UIStyle::GetProperty<int>(const std::string &, const int &) const;
extern template float UIStyle::GetProperty<float>(const std::string &, const float &) const;
extern template bool UIStyle::GetProperty<bool>(const std::string &, const bool &) const;
extern template glm::vec2 UIStyle::GetProperty<glm::vec2>(const std::string &,
                                                          const glm::vec2 &) const;
extern template glm::vec3 UIStyle::GetProperty<glm::vec3>(const std::string &,
                                                          const glm::vec3 &) const;
extern template glm::vec4 UIStyle::GetProperty<glm::vec4>(const std::string &,
                                                          const glm::vec4 &) const;
extern template std::string UIStyle::GetProperty<std::string>(const std::string &,
                                                              const std::string &) const;


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
constexpr const char *BORDER_COLOR = "border.color";

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
constexpr const char *FONT_WEIGHT = "font.weight";
constexpr const char *FONT_STYLE = "font.style";

// 布局相关
constexpr const char *LAYOUT_ALIGN = "layout.align";
constexpr const char *LAYOUT_JUSTIFY = "layout.justify";
constexpr const char *LAYOUT_DIRECTION = "layout.direction";

// 动画相关
constexpr const char *ANIMATION_DURATION = "animation.duration";
constexpr const char *ANIMATION_EASING = "animation.easing";
}  // namespace StyleProperties
}  // namespace mite

#endif  // MITE_UI_STYLE_H
