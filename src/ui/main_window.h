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
   * ������ѭ��.
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
   * ��ʼ��GLFW.
   * 
   * \return false ��ʼ��ʧ��
   */
  bool InitGLFW();

  /**
   * ����GLFW����.
   * 
   * \return false ����ʧ��
   */
  bool CreateGLFWWindow();

  /**
   * ����IMGUI.
   * 
   */
  void SetUpImGUI();
};
}  // namespace mite

#endif