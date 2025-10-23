#ifndef MITE_PROPERTIES_H
#define MITE_PROPERTIES_H

#include "scene_core/scene_core.h"
#include "scene_graph.h"
#include "ui_core/ui_render.h"

namespace mite {
/**
 * @brief 属性基类 - 负责封装属性统一行为
 */
template<typename T> class PropertyBase {
 public:
  /**
   * @brief 更新属性页的组件/数据存储
   */
  void Render(T &component, UIRender &render);
};
}  // namespace mite

#endif  // MITE_PROPERTIES_H
