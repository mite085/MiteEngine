#ifndef MITE_PROPERTIES_H
#  define MITE_PROPERTIES_H

#  include "scene_core/scene_core.h"
#  include "scene_graph.h"
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
  virtual void Update(T& component) = 0;
  virtual void Render(UIRender &render) = 0;
};

/**
 * @brief 变换属性
 */
class TransfromProperty : public PropertyBase<TransformComponent> {
 public:
  TransfromProperty() = default;
  ~TransfromProperty() = default;

  void Update(TransformComponent &component);
  void Render(UIRender &render);

 private:
  Transform::EulerOrder m_EulerOrder;
  glm::vec3 m_WorldPos, m_WorldEulerRot, m_WorldScale;
};
}  // namespace mite

#endif  // MITE_PROPERTIES_H
