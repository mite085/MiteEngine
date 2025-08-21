#ifndef MITE_GLFW_WINDOW
#define MITE_GLFW_WINDOW

#include "glad.h"
#include "glfw/glfw3.h"  // 必须在GLAD加载库之后
#include "glfw_window_callback_adapter.h"
#include "window.h"

namespace mite {
class GLFWWindow : public Window {
 public:
  GLFWWindow();
  virtual ~GLFWWindow() override;

  // 禁用拷贝和移动
  GLFWWindow(const GLFWWindow &) = delete;
  GLFWWindow &operator=(const GLFWWindow &) = delete;

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
  std::pair<double, double> GetMousePosition() const override;

  // 渲染上下文
  void MakeContextCurrent() override;
  void SwapBuffers() override;

  // 窗口数量计数
  static const uint32_t GLFWWindowCount();

 private:
  // 初始化GLFW库（静态）
  static void InitGLFW();
  static void ShutdownGLFW();
  static uint32_t s_GLFWWindowCount;  // 跟踪创建的GLFW窗口数量

 private:
  // GLFW窗口句柄
  GLFWwindow *m_Window = nullptr;

  struct GLFWWindowData {
    std::string title = "Mite Engine";  // 标题
    uint32_t width = 1920;              // 宽度
    uint32_t height = 1080;             // 高度
    bool vsync = false;                 // 垂直同步
    bool fullscreen = false;            // 全屏显示
    bool resizable = true;              // 可拉伸
  } m_WindowData;                       // 窗口回调数据

  // 使用config数据初始化window data
  void InitWindowData(const WindowConfig &config);

  // GLFW回调适配器 - 将GLFW原生事件转换为自定义事件
  GLFWWindowCallbackAdapter m_CallbackAdapter;
};
}  // namespace mite

#endif