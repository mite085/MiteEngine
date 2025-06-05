#ifndef MITE_GLFW_WINDOW
#define MITE_GLFW_WINDOW

#include "headers.h"

#include "glad.h"
#include "GLFW/glfw3.h"  // 必须在GLAD加载库之后
#include "window.h"

namespace mite {

class GLFWWindow : public Window {
 public:
  // 构造函数/析构函数
  explicit GLFWWindow();
  virtual ~GLFWWindow() override;

  // 禁用拷贝和移动
  GLFWWindow(const GLFWWindow &) = delete;
  GLFWWindow &operator=(const GLFWWindow &) = delete;

  // 用于主循环,检测Window关闭标志
  const bool WindowShouldClose() override;

  // 生命周期管理
  void Initialize(const WindowConfig &config) override;
  void Shutdown() override;

  // 窗口属性
  uint32_t GetWidth() const override;
  uint32_t GetHeight() const override;
  void *GetNativeWindow() const override;
  bool IsVSync() const override;

  // 窗口操作
  void SetVSync(bool enabled) override;
  void SetTitle(const std::string &title) override;
  void Resize(uint32_t width, uint32_t height) override;
  void Maximize() override;
  void Minimize() override;
  void Restore() override;
  void Close() override;

  // 事件处理
  void PollEvents() override;
  void WaitEvents() override;

  // 输入处理
  bool IsKeyPressed(int keycode) const override;
  bool IsMouseButtonPressed(int button) const override;
  std::pair<float, float> GetMousePosition() const override;

  // 渲染上下文
  void MakeContextCurrent() override;
  void SwapBuffers() override;

  // 窗口数量计数
  static const uint32_t GLFWWindowCount();
  private:

  // 初始化GLFW库（静态）
  static void InitGLFW();
  static void ShutdownGLFW();
  static uint32_t s_GLFWWindowCount; // 跟踪创建的GLFW窗口数量

  // GLFW回调函数（静态）
  static void ErrorCallback(int error, const char *description);
  static void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
  static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
  static void WindowCloseCallback(GLFWwindow *window);

private:
  // GLFW窗口句柄
  GLFWwindow *m_Window = nullptr;

  struct GLFWWindowData {
    std::string title = "Mite Engine";  // 标题
    uint32_t width = 1280;              // 宽度
    uint32_t height = 720;              // 高度
    bool vsync = false;                 // 垂直同步
    bool fullscreen = false;            // 全屏显示
    bool resizable = true;              // 可拉伸
    GLFWWindow *instance;               // 指向窗口实例
    Window::EventCallbackFn callback;
  } m_WindowData;                       // 窗口回调数据

  // 使用config数据初始化window data
  void InitWindowData(const WindowConfig &config);

  struct {
    double x, y;                           // 鼠标位置
    bool buttons[GLFW_MOUSE_BUTTON_LAST];  // 鼠标按钮状态
    bool keys[GLFW_KEY_LAST];              // 键盘按键状态
  } m_InputState;                          // 输入状态跟踪
};

}  // namespace mite

#endif