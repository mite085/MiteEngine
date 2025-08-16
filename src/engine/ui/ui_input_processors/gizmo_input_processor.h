// gizmo_input_processor.h
#ifndef MITE_GIZMO_INPUT_PROCESSOR
#define MITE_GIZMO_INPUT_PROCESSOR

#include "basic_data/camera.h"
#include "ui_resources/gizmo.h"
#include "input/input_processor.h"
#include "input/input_event.h"

namespace mite {

/**
 * @brief Gizmo输入处理器
 *
 * 修改点：
 * - 不再直接操作Entity
 * - 使用glm::mat4作为变换矩阵
 */
class GizmoInputProcessor : public InputProcessor {
 public:
  GizmoInputProcessor(std::shared_ptr<Camera> camera, glm::mat4 &transformMatrix);

  // InputProcessor接口
  int GetPriority() const override
  {
    return InputPriority::GIZMO;
  }
  const std::string GetID() const override
  {
    static const std::string id = "GizmoProcessor";
    return id;
  }

  // 每帧更新
  void Update(float deltaTime);

  // 配置方法
  void SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size);
  void SetTransform(glm::mat4 &transformMatrix);

  // 处理事件的方法
  bool HandleEvent(Event &e) override
  {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DISPATCH_FN(handleMouseButtonPressed));
    dispatcher.Dispatch<KeyPressedEvent>(BIND_DISPATCH_FN(handleKeyPressed));
    return e.handled;
  }

 protected:
  // 事件处理
  bool handleMouseButtonPressed(MouseButtonPressedEvent &e);
  bool handleKeyPressed(KeyPressedEvent &e);

 private:
  std::shared_ptr<Camera> m_Camera;
  glm::mat4 &m_TransformMatrix;  // 引用外部变换矩阵
  Gizmo m_Gizmo;
  glm::vec2 m_ViewportPos;
  glm::vec2 m_ViewportSize;
};

}  // namespace mite

#endif