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
 * ImGui内部有一套完整的事件处理逻辑：
 * 1. ImGui::GetIO().AddKeyEvent(mouseKey, true);阶段收集事件
 * 2. ImGui::NewFrame();时处理所有累积的输入事件
 * 3. ImGui::Begin("My Panel");时进行命中测试和事件分发（按照绘制的顺序从后绘制到先绘制进行传播）
 * 4. if (ImGui::Button("Click Me")){ 这里的Button()返回值即为鼠标点击的结果
 * 5. OnButtonClicked();}内部可以自定义处理函数，执行鼠标点击之后的逻辑
 * 6. ImGui::End();，ImGui::Render();作为Begin和NewFrame的结束标志
 * 所以InputContext仅负责将鼠标键盘输入传递给Adapter即可，ImGui会处理好这一切
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
  void ProcessEvent(Event &e);

  // 初始化方法
  void Initialize();
  void Shutdown();

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
  void ProcessMouseMoveEvent(MouseMoveEvent &e);
  void ProcessMouseButtonEvent(MouseButtonPressedEvent &e);
  void ProcessMouseButtonEvent(MouseButtonReleasedEvent &e);
  void ProcessMouseScrollEvent(MouseScrollEvent &e);
  void ProcessKeyEvent(KeyPressedEvent &e);
  void ProcessKeyEvent(KeyReleasedEvent &e);
  void ProcessKeyTypedEvent(KeyTypedEvent &e);

  // GLFW键码到ImGuiKey的转换
  ImGuiKey ConvertGlfwKeyToImGuiKey(int glfwKey);
  ImGuiKey ConvertGlfwMouseButtonToImGuiKey(int glfwButton);

  Logger m_Logger;

  // 输入状态跟踪
  glm::vec2 m_LastMousePos = glm::vec2(0);

  // 显示相关状态
  glm::ivec2 m_DisplaySize = glm::ivec2(0);
  glm::vec2 m_FramebufferScale = glm::vec2(1.0f);
};

}  // namespace mite

#endif  // MITE_IMGUI_INPUT_ADAPTER_H
