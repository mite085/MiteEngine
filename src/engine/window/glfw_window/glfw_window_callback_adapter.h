#ifndef MITE_GLFW_WINDOW_CALLBACK_ADAPTER
#define MITE_GLFW_WINDOW_CALLBACK_ADAPTER

#include "headers/headers.h"

namespace mite {
/**
 * @brief GLFW回调适配器 - 将GLFW原生事件转换为自定义事件
 *
 * 负责所有GLFW相关事件（窗口、输入等）的转换和转发
 */
class GLFWWindowCallbackAdapter : public CallbackAdapter<GLFWwindow *> {
 public:
  explicit GLFWWindowCallbackAdapter();

  ~GLFWWindowCallbackAdapter() override;

  /**
   * @brief 注册所有回调到原始系统
   * @param source 原始系统对象指针
   */
  void RegisterCallbacks(GLFWwindow *window) override;
  /**
   * @brief 注销所有回调
   */
  void UnregisterCallbacks() override;

  /**
   * @brief 运行错误回调函数
   * @param error 错误编号
   * @param description 错误描述
   */
  static void ErrorCallback(int error, const char *description);

 private:
  // 窗口事件处理函数  ==============================================
  /**
   * @brief 处理窗口关闭事件
   */
  static void HandleWindowClose(GLFWwindow *window);

  /**
   * @brief 处理窗口大小改变事件
   * @param width 新宽度（像素）
   * @param height 新高度（像素）
   */
  static void HandleWindowResize(GLFWwindow *window, int width, int height);

  /**
   * @brief 处理窗口焦点变化事件
   * @param focused GLFW_TRUE表示获得焦点，GLFW_FALSE表示失去焦点
   */
  static void HandleWindowFocus(GLFWwindow *window, int focused);

  /**
   * @brief 处理窗口移动事件
   * @param xpos 新X坐标（屏幕坐标）
   * @param ypos 新Y坐标（屏幕坐标）
   */
  static void HandleWindowMoved(GLFWwindow *window, int xpos, int ypos);

  /* 鼠标事件处理函数 */

  /**
   * @brief 处理鼠标移动事件
   * @param xpos 鼠标X坐标（窗口坐标）
   * @param ypos 鼠标Y坐标（窗口坐标）
   */
  static void HandleMouseMove(GLFWwindow *window, double xpos, double ypos);

  /**
   * @brief 处理鼠标按钮事件
   * @param button 按钮编号（GLFW_MOUSE_BUTTON_*）
   * @param action 动作（GLFW_PRESS/GLFW_RELEASE）
   * @param mods 修饰键（GLFW_MOD_*组合）
   */
  static void HandleMouseButton(GLFWwindow *window, int button, int action, int mods);

  /**
   * @brief 处理鼠标滚轮事件
   */
  static void HandleMouseScroll(GLFWwindow *window, double xoffset, double yoffset);

  /* 键盘事件处理函数 */

  /**
   * @brief 处理键盘按键事件
   * @param key 键位编号（GLFW_KEY_*）
   * @param scancode 物理键位扫描码
   * @param action 动作（GLFW_PRESS/GLFW_RELEASE/GLFW_REPEAT）
   * @param mods 修饰键（GLFW_MOD_*组合）
   */
  static void HandleKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods);

  /**
   * @brief 处理字符输入事件（用于文本输入）
   * @param codepoint Unicode码点
   */
  static void HandleCharInput(GLFWwindow *window, unsigned int codepoint);

  /**
   * @brief 安全获取适配器实例
   */
  static GLFWWindowCallbackAdapter *GetAdapter(GLFWwindow *window);

  // GLFW窗口句柄
  GLFWwindow *m_Window = nullptr;

  static Logger s_Logger;  // 静态日志对象
};
};  // namespace mite

#endif
