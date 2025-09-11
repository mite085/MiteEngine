#ifndef MITE_UI_LAYOUT_H
#define MITE_UI_LAYOUT_H

#include "ui_element/ui_element.h"

namespace mite {
/**
 * @brief 布局算法基类，定义布局接口
 */
class UILayout {
 public:
  // 对齐方式枚举
  enum class Alignment {
    TopLeft,       // 左上
    TopCenter,     // 中上
    TopRight,      // 右上
    CenterLeft,    // 左中
    Center,        // 中心
    CenterRight,   // 右中
    BottomLeft,    // 左下
    BottomCenter,  // 中下
    BottomRight    // 右下
  };
  // 布局类型枚举
  enum class LayoutType {
    Horizontal,  // 水平布局
    Vertical     // 垂直布局
  };

  /**
   * @brief 根据类型创建布局
   * @param type
   * @return
   */
  static std::shared_ptr<UILayout> CreateUILayout(LayoutType type, Alignment alignment);

  UILayout(Alignment alignment) : m_Alignment(alignment) {}
  virtual ~UILayout() = default;

  /**
   * @brief 计算布局
   * @param elements 需要布局的元素列表
   * @param containerSize 容器尺寸
   * @param containerPosition 容器位置
   * @return 布局后的元素位置信息
   */
  virtual std::vector<glm::vec2> CalculateLayout(
      const std::vector<std::shared_ptr<UIElement>> &elements,
      const glm::vec2 &containerSize,
      const glm::vec2 &containerPosition) = 0;

  /**
   * @brief 获取布局类型名称
   */
  virtual const char *GetLayoutType() const = 0;

  /**
   * @brief 设置间距
   */
  virtual void SetSpacing(float spacing);

  /**
   * @brief 获取间距
   */
  float GetSpacing() const;

  /**
   * @brief 设置边距
   */
  virtual void SetPadding(const glm::vec4 &padding);

  /**
   * @brief 获取边距
   */
  glm::vec4 GetPadding() const;

  /**
   * @brief 设置是否拉伸子元素以适应容器
   */
  virtual void SetStretchChildren(bool stretch);

  /**
   * @brief 获取是否拉伸子元素
   */
  bool GetStretchChildren() const;

  /**
   * @brief 设置对齐方式
   */
  void SetAlignment(Alignment alignment);

  /**
   * @brief 获取对齐方式
   */
  Alignment GetAlignment() const;

  /**
   * @brief 克隆布局对象
   */
  virtual std::shared_ptr<UILayout> Clone() const = 0;

 protected:
  float m_Spacing = 5.0f;                      // 元素间距
  glm::vec4 m_Padding = {0, 0, 0, 0};          // 边距: left, top, right, bottom
  bool m_StretchChildren = false;              // 是否拉伸子元素
  Alignment m_Alignment = Alignment::TopLeft;  // 对齐方式，默认从左到右 + 从上往下
};
}  // namespace mite

#endif  // MITE_UI_LAYOUT_H
