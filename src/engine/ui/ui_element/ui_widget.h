#ifndef MITE_UI_WIDGET_H
#define MITE_UI_WIDGET_H

#include "ui_core/ui_style.h"
#include "ui_element.h"

namespace mite {

/**
 * @brief UI控件基类，所有UI控件的抽象基类
 * 
 */
class UIWidget : public UIElement {
 public:
  explicit UIWidget(const std::string &name) : UIElement(name){}
  virtual ~UIWidget() = default;

  // 针对Button等无需每帧update的控件，提供无需重写的空函数
  virtual void Update(float deltaTime){};
  virtual void Render() = 0;
};

}  // namespace mite

#endif  // MITE_UI_WIDGET_H
