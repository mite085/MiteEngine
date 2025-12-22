#ifndef MITE_UI_IMGUI_INPUT_PRODUCER_H
#define MITE_UI_IMGUI_INPUT_PRODUCER_H

#include "imgui.h"
#include "input/input_event.h"

namespace mite {
/**
 * @brief ImGui输入事件生产者
 *
 * 职责：从ImGui获取所有鼠标和键盘输入状态，生成标准输入事件
 * 特点：统一处理主窗口和独立ImGui窗口的输入，解决GLFW无法处理独立窗口的问题
 * 调用时机：每帧在ImGui::NewFrame()之后调用ProduceInputEvents()
 */
class UIImguiInputProducer {
 public:
  /**
   * @brief 生产输入事件
   *
   * 从ImGui输入状态生成标准鼠标和键盘事件
   * 只在输入状态变化时生成事件，避免重复事件
   *
   * 调用位置：每帧在ImGui::NewFrame()之后调用
   */
  static void ProduceInputEvents();

 private:
  /**
   * @brief 生产鼠标移动事件
   *
   * 只在鼠标位置变化时生成MouseMoveEvent
   * 使用静态变量记录上次位置，避免重复事件
   */
  static void ProduceMouseMoveEvents();

  /**
   * @brief 生产鼠标按键事件
   *
   * 监测鼠标按键状态变化，生成按下和释放事件
   * 支持所有ImGui鼠标按钮（左键、右键、中键等）
   */
  static void ProduceMouseButtonEvents();

  /**
   * @brief 生产键盘按键事件
   *
   * 监测键盘按键状态变化，生成按下和释放事件
   * 将ImGui键值映射为GLFW键值以保持兼容性
   */
  static void ProduceKeyboardEvents();

  /**
   * @brief 生产鼠标滚轮事件
   *
   * 监测滚轮变化，生成MouseScrollEvent
   * 支持水平和垂直滚轮
   */
  static void ProduceMouseScrollEvents();

  /**
   * @brief 将ImGui键值映射为GLFW键值
   *
   * @param imguiKey ImGui键值（ImGuiKey_*）
   * @return int 对应的GLFW键值，如果无对应映射返回-1
   */
  static int MapImGuiKeyToGLFW(int imguiKey);

  // 静态状态记录，用于检测状态变化
  static ImVec2 s_LastMousePos;                            // 上次鼠标位置
  static bool s_LastMouseButtons[ImGuiMouseButton_COUNT];  // 上次鼠标按键状态
  static bool s_LastKeys[ImGuiKey_COUNT];                  // 上次键盘按键状态
};
}  // namespace mite

#endif  // MITE_UI_IMGUI_INPUT_PRODUCER_H
