#ifndef MITE_VIEWPORT_INPUT_PROCESSOR
#define MITE_VIEWPORT_INPUT_PROCESSOR

#include "camera_input_processor.h"

namespace mite {
/**
 * @brief  ViewportInputProcessor 编辑器视口输入事件处理器
 *
 * 定位：编辑器视口专用的交互逻辑（如视口导航、网格吸附、视角预设）。
 * 职责：整合相机控制 + 编辑器特有行为（如右键拖拽导航、Alt+左键特殊操作）。
 * 复用性：仅限编辑器使用，依赖编辑器状态（如当前工具模式）。
 * 输入优先级：InputPriority::UI_FORM (500)，高于通用相机但低于模态UI。
 */
class ViewportInputProcessor : public CameraInputProcessor {
 public:
  explicit ViewportInputProcessor(std::shared_ptr<Camera> camera,
                                  int navigationButton);

  // InputProcessor接口
  const std::string &GetID() const override
  {
    return "ViewportNavigation";
  }
  int GetPriority() const override
  {
    return InputPriority::UI_FORM;
  }

  // 配置方法
  void SetNavigationButton(int button)
  {
    m_NavigationButton = button;
  }
  void SetViewportHovered(bool hovered)
  {
    m_ViewportHovered = hovered;
  }
  void SetViewportFocused(bool focused)
  {
    m_ViewportFocused = focused;
  }

 protected:
  // 重写事件处理方法，增加视口状态检查
  bool handleMouseMove(MouseMoveEvent &e) override;
  bool handleMouseButton(MouseButtonReleasedEvent &e) override;
  bool handleMouseScroll(MouseScrollEvent &e) override;
  bool handleKeyEvent(KeyReleasedEvent &e) override;

 private:
  int m_NavigationButton;
  bool m_ViewportHovered = false;
  bool m_ViewportFocused = false;
};
}  // namespace mite

#endif