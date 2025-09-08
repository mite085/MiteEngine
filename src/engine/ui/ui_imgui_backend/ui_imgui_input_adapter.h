#ifndef MITE_IMGUI_INPUT_ADAPTER_H
#define MITE_IMGUI_INPUT_ADAPTER_H

#include "input/input_event.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
namespace mite {
/**
 * @brief ImGui输入适配器 - 负责ImGui与引擎输入系统的桥接
 *
 * 功能：
 * 1. 输入事件桥接：将引擎输入事件转换为ImGui IO状态
 * 2. 输入优先级管理：作为最高优先级的输入上下文，根据ImGui状态决定是否阻止事件传递
 * 3. 输入状态同步：维护ImGui输入状态与引擎输入状态的一致性
 */
class ImGuiInputAdapter{
 public:
  ImGuiInputAdapter();
  ~ImGuiInputAdapter();

  // InputContext接口
  bool ProcessEvent(Event &e);

  // 初始化方法
  void Initialize();
  void Shutdown();

  // 每帧更新ImGui IO状态
  void UpdateImGuiIO();

  // 显示尺寸和缩放相关方法
  void UpdateDisplaySize(GLFWwindow *window);
  void UpdateFramebufferScale(GLFWwindow *window);

  glm::vec2 GetDisplaySize() const
  {
    return m_DisplaySize;
  }
  glm::vec2 GetFramebufferScale() const
  {
    return m_FramebufferScale;
  }

 private:
  // 具体事件处理方法
  bool ProcessMouseMoveEvent(MouseMoveEvent &e);
  bool ProcessMouseButtonEvent(MouseButtonPressedEvent &e);
  bool ProcessMouseButtonEvent(MouseButtonReleasedEvent &e);
  bool ProcessMouseScrollEvent(MouseScrollEvent &e);
  bool ProcessKeyEvent(KeyPressedEvent &e);
  bool ProcessKeyEvent(KeyReleasedEvent &e);
  bool ProcessKeyTypedEvent(KeyTypedEvent &e);

  // GLFW键码到ImGuiKey的转换
  ImGuiKey ConvertGlfwKeyToImGuiKey(int glfwKey);
  ImGuiKey ConvertGlfwMouseButtonToImGuiKey(int glfwButton);

  Logger m_Logger;

  // 输入状态跟踪
  glm::vec2 m_LastMousePos;

  // 显示相关状态
  glm::ivec2 m_DisplaySize = glm::ivec2(0);
  glm::vec2 m_FramebufferScale = glm::vec2(1.0f);
};

}  // namespace mite

#endif  // MITE_IMGUI_INPUT_ADAPTER_H
