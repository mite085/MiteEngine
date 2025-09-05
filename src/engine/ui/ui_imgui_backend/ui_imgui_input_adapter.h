#ifndef MITE_IMGUI_INPUT_ADAPTER_H
#define MITE_IMGUI_INPUT_ADAPTER_H

#include "GLFW/glfw3.h"
#include "ui_imgui_backend/ui_imgui_backend.h"
#include "input/modular_input_context.h"

namespace mite {

/**
 * @brief ImGui输入适配器 - 负责ImGui与引擎输入系统的桥接
 *
 * 功能：
 * 1. 输入事件桥接：将引擎输入事件转换为ImGui IO状态
 * 2. 输入优先级管理：作为最高优先级的输入上下文，根据ImGui状态决定是否阻止事件传递
 * 3. 输入状态同步：维护ImGui输入状态与引擎输入状态的一致性
 * 
 * 注意：
 * 虽然ModularInputContext维护了InputProcessor列表，
 * 但ImGuiInputAdapter无需实现InputProcessor。 
 * 因为ImGuiInputAdapter本身就是一个完整的输入上下文，
 * 它处理所有ImGui相关的输入转换，功能集中且单一，
 * 多个处理器会增加复杂度，而单一职责更清晰
 */
class ImGuiInputAdapter : public ModularInputContext {
 public:
  explicit ImGuiInputAdapter(const std::string &name);
  ~ImGuiInputAdapter();

  // InputContext接口
  bool ProcessEvent(Event &e) override;

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
