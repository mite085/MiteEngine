#ifndef MITE_UI_LAYOUT_HORIZONTAL_H
#define MITE_UI_LAYOUT_HORIZONTAL_H

#include "ui_layout.h"

namespace mite {

/**
 * @brief 水平布局算法
 */
class UILayoutHorizontal : public UILayout {
 public:
  explicit UILayoutHorizontal(Alignment alignment) : UILayout(alignment) {}
  ~UILayoutHorizontal() override = default;

  /**
   * @brief 计算水平布局
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
};

}  // namespace mite

#endif  // MITE_UI_LAYOUT_HORIZONTAL_H
