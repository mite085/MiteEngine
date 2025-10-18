#ifndef MITE_VIEWPORT_INPUT_CONTEXT_H
#define MITE_VIEWPORT_INPUT_CONTEXT_H

#include "input/input_context.h"

namespace mite {

/**
 * @brief 视口输入上下文 - 处理ViewportPanel的鼠标交互
 *
 * 职责：
 * 1. 管理视口焦点状态
 * 2. 处理鼠标在视口内的交互
 * 3. 支持Node选择和拖动操作
 * 4. 与GizmoOverlay协同工作
 */
class ViewportInputContext : public InputContext {
 public:
  explicit ViewportInputContext(const std::string &name);
  virtual ~ViewportInputContext() = default;

  // 视口状态管理
  void SetViewportFocus(bool focused);
  bool IsViewportFocused() const { return m_ViewportFocused; }

  void SetViewportRect(const glm::vec2 &pos, const glm::vec2 &size);
  bool IsMouseInViewport(const glm::vec2 &mousePos) const;

 protected:
  // 输入事件处理
  void ProcessMouseMoveEvent(MouseMoveEvent &e) override;
  void ProcessMouseButtonPressedEvent(MouseButtonPressedEvent &e) override;
  void ProcessMouseButtonReleasedEvent(MouseButtonReleasedEvent &e) override;
  void ProcessMouseScrollEvent(MouseScrollEvent &e) override;
  void ProcessKeyPressdEvent(KeyPressedEvent &e) override;
  void ProcessKeyReleasedEvent(KeyReleasedEvent &e) override;
  void ProcessKeyTypedEvent(KeyTypedEvent &e) override;

 private:
  // 视口状态
  bool m_ViewportFocused = false;
  glm::vec2 m_ViewportPos = {0, 0};
  glm::vec2 m_ViewportSize = {0, 0};

  // 交互状态
  bool m_IsDragging = false;
  glm::vec2 m_LastMousePos = {0, 0};
  glm::vec2 m_DragStartPos = {0, 0};

  // 内部方法
  glm::vec2 ScreenToViewport(const glm::vec2 &screenPos) const;
  void HandleNodeSelection(const glm::vec2 &viewportPos);
  void HandleNodeDrag(const glm::vec2 &viewportPos, const glm::vec2 &delta);
};

}  // namespace mite

#endif  // MITE_VIEWPORT_INPUT_CONTEXT_H
