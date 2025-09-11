#ifndef MITE_UI_LAYOUT_VERTICAL_H
#define MITE_UI_LAYOUT_VERTICAL_H

#include "ui_layout.h"

namespace mite {

/**
 * @brief 垂直布局算法
 */
class UILayoutVertical : public UILayout {
 public:
  explicit UILayoutVertical(Alignment alignment) : UILayout(alignment) {}
  ~UILayoutVertical() override = default;

  /**
   * @brief 计算垂直布局
   */
  std::vector<glm::vec2> CalculateLayout(const std::vector<std::shared_ptr<UIElement>> &elements,
                                         const glm::vec2 &containerSize,
                                         const glm::vec2 &containerPosition) override;

  /**
   * @brief 获取布局类型名称
   */
  const char *GetLayoutType() const override;

  /**
   * @brief 克隆布局对象
   */
  std::shared_ptr<UILayout> Clone() const override;

 private:
  Alignment m_Alignment = Alignment::TopLeft;  // 对齐方式
};
}  // namespace mite

#endif  // MITE_UI_LAYOUT_VERTICAL_H
