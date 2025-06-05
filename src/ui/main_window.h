#ifndef MITE_MAINWINDOW
#define MITE_MAINWINDOW

#include <GLFW/glfw3.h> 
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


namespace mite {
class MainWindow {
 public:
  MainWindow();
  ~MainWindow();
  /**
   * 窗口主循环.
   * 
   */
  void show();

 private:
  GLFWwindow *m_window;
  
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
 private:
  /**
   * 初始化GLFW.
   * 
   * \return false 初始化失败
   */
  bool InitGLFW();

  /**
   * 创建GLFW窗口.
   * 
   * \return false 创建失败
   */
  bool CreateGLFWWindow();

  /**
   * 启动IMGUI.
   * 
   */
  void SetUpImGUI();
};
}  // namespace mite

#endif