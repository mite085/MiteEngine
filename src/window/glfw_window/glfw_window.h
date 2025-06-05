#ifndef MITE_GLFW_WINDOW
#define MITE_GLFW_WINDOW

#include "headers.h"

#include "glad.h"
#include "GLFW/glfw3.h"  // ������GLAD���ؿ�֮��
#include "window.h"

namespace mite {

class GLFWWindow : public Window {
 public:
  // ���캯��/��������
  explicit GLFWWindow();
  virtual ~GLFWWindow() override;

  // ���ÿ������ƶ�
  GLFWWindow(const GLFWWindow &) = delete;
  GLFWWindow &operator=(const GLFWWindow &) = delete;

  // ������ѭ��,���Window�رձ�־
  const bool WindowShouldClose() override;

  // �������ڹ���
  void Initialize(const WindowConfig &config) override;
  void Shutdown() override;

  // ��������
  uint32_t GetWidth() const override;
  uint32_t GetHeight() const override;
  void *GetNativeWindow() const override;
  bool IsVSync() const override;

  // ���ڲ���
  void SetVSync(bool enabled) override;
  void SetTitle(const std::string &title) override;
  void Resize(uint32_t width, uint32_t height) override;
  void Maximize() override;
  void Minimize() override;
  void Restore() override;
  void Close() override;

  // �¼�����
  void PollEvents() override;
  void WaitEvents() override;

  // ���봦��
  bool IsKeyPressed(int keycode) const override;
  bool IsMouseButtonPressed(int button) const override;
  std::pair<float, float> GetMousePosition() const override;

  // ��Ⱦ������
  void MakeContextCurrent() override;
  void SwapBuffers() override;

  // ������������
  static const uint32_t GLFWWindowCount();
  private:

  // ��ʼ��GLFW�⣨��̬��
  static void InitGLFW();
  static void ShutdownGLFW();
  static uint32_t s_GLFWWindowCount; // ���ٴ�����GLFW��������

  // GLFW�ص���������̬��
  static void ErrorCallback(int error, const char *description);
  static void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
  static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
  static void WindowCloseCallback(GLFWwindow *window);

private:
  // GLFW���ھ��
  GLFWwindow *m_Window = nullptr;

  struct GLFWWindowData {
    std::string title = "Mite Engine";  // ����
    uint32_t width = 1280;              // ���
    uint32_t height = 720;              // �߶�
    bool vsync = false;                 // ��ֱͬ��
    bool fullscreen = false;            // ȫ����ʾ
    bool resizable = true;              // ������
    GLFWWindow *instance;               // ָ�򴰿�ʵ��
    Window::EventCallbackFn callback;
  } m_WindowData;                       // ���ڻص�����

  // ʹ��config���ݳ�ʼ��window data
  void InitWindowData(const WindowConfig &config);

  struct {
    double x, y;                           // ���λ��
    bool buttons[GLFW_MOUSE_BUTTON_LAST];  // ��갴ť״̬
    bool keys[GLFW_KEY_LAST];              // ���̰���״̬
  } m_InputState;                          // ����״̬����
};

}  // namespace mite

#endif